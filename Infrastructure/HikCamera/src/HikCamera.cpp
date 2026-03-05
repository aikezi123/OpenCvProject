#include "HikCamera.h"
#include "MvCameraControl.h" 
#include <iostream>

// ====================================================
// 1. 全局回调函数：极简瘦身！只做一件事：转发给类的内部函数
// ====================================================
static void __stdcall GlobalImageCallback(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    if (pUser) {
        // 彻底抛弃 OpenCV 手动解码，直接丢给带有内存池和官方 ISP 的 processAndTrigger！
        static_cast<HikCamera*>(pUser)->processAndTrigger(pData, pFrameInfo);
    }
}

// ====================================================
// 2. 构造、析构与流控制
// ====================================================
HikCamera::HikCamera() : m_handle(nullptr), m_isStreaming(false) {}

HikCamera::~HikCamera() {
    closeDevice();
}

void HikCamera::registerFrameCallback(FrameCallback callback) {
    m_callback = callback;
}

void HikCamera::triggerCallback(const cv::Mat& frame) {
    if (m_callback) m_callback(frame);
}

CameraStatus HikCamera::startStream() {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;

    MV_CC_RegisterImageCallBackEx(m_handle, GlobalImageCallback, this);

    if (MV_CC_StartGrabbing(m_handle) == MV_OK) {
        m_isStreaming = true;
        return CameraStatus::SUCCESS;
    }
    return CameraStatus::STREAM_FAILED;
}

CameraStatus HikCamera::stopStream() {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;

    // 拔掉回调水管，防止退出时崩溃
    MV_CC_RegisterImageCallBackEx(m_handle, nullptr, nullptr);

    if (MV_CC_StopGrabbing(m_handle) == MV_OK) {
        m_isStreaming = false;
        return CameraStatus::SUCCESS;
    }
    return CameraStatus::STREAM_FAILED;
}

// ====================================================
// 3. 设备连接与初始化 (纯 USB 极速版)
// ====================================================
std::vector<CameraInfo> HikCamera::enumDevices() {
    std::vector<CameraInfo> cameraList;
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    if (MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList) != MV_OK) return cameraList;

    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pInfo = stDeviceList.pDeviceInfo[i];
        if (pInfo->nTLayerType == MV_USB_DEVICE) {
            CameraInfo info;
            info.serialNumber = reinterpret_cast<char*>(pInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            info.modelName = reinterpret_cast<char*>(pInfo->SpecialInfo.stUsb3VInfo.chModelName);
            info.ipAddress = "USB3.0 Direct Connection";
            cameraList.push_back(info);
        }
    }
    return cameraList;
}

CameraStatus HikCamera::openDevice(const std::string& serialNumber) {
    if (m_handle != nullptr) return CameraStatus::SUCCESS;

    MV_CC_DEVICE_INFO_LIST stDeviceList;
    if (MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList) != MV_OK || stDeviceList.nDeviceNum == 0) {
        return CameraStatus::DEVICE_NOT_FOUND;
    }

    int targetIndex = 0; // 默认第一台
    if (!serialNumber.empty()) {
        targetIndex = -1;
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
            if (stDeviceList.pDeviceInfo[i]->nTLayerType == MV_USB_DEVICE) {
                if (std::string(reinterpret_cast<char*>(stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chSerialNumber)) == serialNumber) {
                    targetIndex = i;
                    break;
                }
            }
        }
        if (targetIndex == -1) return CameraStatus::DEVICE_NOT_FOUND;
    }

    // A. 创建句柄
    if (MV_CC_CreateHandle(&m_handle, stDeviceList.pDeviceInfo[targetIndex]) != MV_OK) return CameraStatus::OPEN_FAILED;

    // B. 【Bug修复】：必须先打开设备，才能设置参数！
    if (MV_CC_OpenDevice(m_handle) != MV_OK) {
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
        return CameraStatus::OPEN_FAILED;
    }

    // C. 强设核心参数 (连续曝光、自动白平衡、关闭触发)
    MV_CC_SetEnumValue(m_handle, "ExposureAuto", 0);
    MV_CC_SetEnumValue(m_handle, "BalanceWhiteAuto", 2);
    MV_CC_SetEnumValue(m_handle, "TriggerMode", 0);

    return CameraStatus::SUCCESS;
}

CameraStatus HikCamera::closeDevice() {
    if (m_handle != nullptr) {
        if (m_isStreaming) stopStream();
        MV_CC_CloseDevice(m_handle);
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
    }
    return CameraStatus::SUCCESS;
}

// ====================================================
// 4. 核心图像转码引擎 (带锁与动态内存池)
// ====================================================
void HikCamera::processAndTrigger(unsigned char* pData, void* pInfo) {
    MV_FRAME_OUT_INFO_EX* pFrameInfo = static_cast<MV_FRAME_OUT_INFO_EX*>(pInfo);

    // 互斥锁：防止多线程导致画面撕裂
    std::lock_guard<std::mutex> lock(m_mutex);

    if (pFrameInfo->enPixelType == PixelType_Gvsp_Mono8) {
        cv::Mat monoMat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);
        triggerCallback(monoMat.clone());
        return;
    }

    // 动态维护复用缓存池
    unsigned int requiredSize = pFrameInfo->nWidth * pFrameInfo->nHeight * 3;
    if (m_convertBuf.size() < requiredSize) {
        m_convertBuf.resize(requiredSize);
    }

    // 调用海康官方 ISP 无损还原真彩色
    MV_CC_PIXEL_CONVERT_PARAM stConvertParam = { 0 };
    stConvertParam.nWidth = pFrameInfo->nWidth;
    stConvertParam.nHeight = pFrameInfo->nHeight;
    stConvertParam.pSrcData = pData;
    stConvertParam.nSrcDataLen = pFrameInfo->nFrameLen;
    stConvertParam.enSrcPixelType = pFrameInfo->enPixelType;
    stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
    stConvertParam.pDstBuffer = m_convertBuf.data();
    stConvertParam.nDstBufferSize = m_convertBuf.size();

    if (MV_CC_ConvertPixelType(m_handle, &stConvertParam) == MV_OK) {
        cv::Mat rgbMat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, m_convertBuf.data());
        triggerCallback(rgbMat.clone()); // 深拷贝发送，保命机制
    }
}

// ====================================================
// 5. 核心参数控制 (真正调用海康 SDK 操作硬件)
// ====================================================

CameraStatus HikCamera::setExposureTime(float timeUs) {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;

    // 调用海康 SDK 写入曝光时间
    int nRet = MV_CC_SetFloatValue(m_handle, "ExposureTime", timeUs);
    if (nRet == MV_OK) {
        return CameraStatus::SUCCESS;
    }
    std::cerr << "[HikCamera] 设置曝光时间失败! 错误码: " << std::hex << nRet << std::endl;
    return CameraStatus::PARAM_SET_FAILED;
}

float HikCamera::getExposureTime() {
    if (m_handle == nullptr) return 0.0f;

    MVCC_FLOATVALUE stParam = { 0 };
    int nRet = MV_CC_GetFloatValue(m_handle, "ExposureTime", &stParam);
    if (nRet == MV_OK) {
        return stParam.fCurValue;
    }
    return 0.0f;
}

CameraStatus HikCamera::setGain(float gain) {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;

    // 调用海康 SDK 写入增益
    int nRet = MV_CC_SetFloatValue(m_handle, "Gain", gain);
    if (nRet == MV_OK) {
        return CameraStatus::SUCCESS;
    }
    std::cerr << "[HikCamera] 设置增益失败! 错误码: " << std::hex << nRet << std::endl;
    return CameraStatus::PARAM_SET_FAILED;
}

float HikCamera::getGain() {
    if (m_handle == nullptr) return 0.0f;

    MVCC_FLOATVALUE stParam = { 0 };
    int nRet = MV_CC_GetFloatValue(m_handle, "Gain", &stParam);
    if (nRet == MV_OK) {
        return stParam.fCurValue;
    }
    return 0.0f;
}

float HikCamera::getMaxGain() {
    if (m_handle == nullptr) return 0.0f;

    MVCC_FLOATVALUE stParam = { 0 };
    int nRet = MV_CC_GetFloatValue(m_handle, "Gain", &stParam);
    if (nRet == MV_OK) {
        return stParam.fMax; // 拿取底层结构体里的最大值！
    }
    return 12.0f; // 如果获取失败，给个保守的保底值
}

// 纯异步架构下，同步抓图已被废弃，保留空实现以满足接口契约
bool HikCamera::grabFrame(cv::Mat& outFrame, int timeoutMs) {
    return false;
}