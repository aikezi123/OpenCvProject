// Infrastructure/HikCamera/src/HikCamera.cpp

#include "HikCamera.h"
#include <iostream>

// 在 cpp 文件中包含海康的头文件，彻底防止污染上层
#include "MvCameraControl.h" 

HikCamera::HikCamera() : m_handle(nullptr), m_isStreaming(false) {
}

HikCamera::~HikCamera() {
    // 析构时确保安全关闭
    closeDevice();
}

std::vector<CameraInfo> HikCamera::enumDevices() {
    std::vector<CameraInfo> cameraList;
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // 枚举千兆网口和 USB 接口的相机
    int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (nRet != MV_OK) {
        return cameraList;
    }

    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        CameraInfo info;

        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) {
            info.serialNumber = (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber;
            info.modelName = (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName;

            // 解析 IP 地址
            int ip1 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int ip2 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int ip3 = ((pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int ip4 = (pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
            info.ipAddress = std::to_string(ip1) + "." + std::to_string(ip2) + "." +
                std::to_string(ip3) + "." + std::to_string(ip4);
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE) {
            info.serialNumber = (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber;
            info.modelName = (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName;
            info.ipAddress = "USB";
        }
        cameraList.push_back(info);
    }
    return cameraList;
}

CameraStatus HikCamera::openDevice(const std::string& serialNumber) {
    if (m_handle != nullptr) {
        return CameraStatus::SUCCESS; // 已经打开了
    }

    MV_CC_DEVICE_INFO_LIST stDeviceList;
    MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);

    if (stDeviceList.nDeviceNum == 0) {
        return CameraStatus::DEVICE_NOT_FOUND;
    }

    // 这里简化逻辑：如果不传序列号，默认打开第一台设备
    int deviceIndex = 0;

    // 1. 创建句柄
    int nRet = MV_CC_CreateHandle(&m_handle, stDeviceList.pDeviceInfo[deviceIndex]);
    if (nRet != MV_OK) return CameraStatus::OPEN_FAILED;

    // 2. 打开设备
    nRet = MV_CC_OpenDevice(m_handle);
    if (nRet != MV_OK) {
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
        return CameraStatus::OPEN_FAILED;
    }

    // 设置为触发模式 OFF（连续自由抓图模式）
    MV_CC_SetEnumValue(m_handle, "TriggerMode", 0);

    return CameraStatus::SUCCESS;
}

CameraStatus HikCamera::closeDevice() {
    if (m_handle != nullptr) {
        if (m_isStreaming) {
            stopStream();
        }
        MV_CC_CloseDevice(m_handle);
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
    }
    return CameraStatus::SUCCESS;
}

CameraStatus HikCamera::startStream() {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;

    int nRet = MV_CC_StartGrabbing(m_handle);
    if (nRet == MV_OK) {
        m_isStreaming = true;
        return CameraStatus::SUCCESS;
    }
    return CameraStatus::STREAM_FAILED;
}

CameraStatus HikCamera::stopStream() {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;

    int nRet = MV_CC_StopGrabbing(m_handle);
    if (nRet == MV_OK) {
        m_isStreaming = false;
        return CameraStatus::SUCCESS;
    }
    return CameraStatus::STREAM_FAILED;
}

// ==========================================
// 核心：同步拉取一帧图像并转为 OpenCV Mat
// ==========================================
bool HikCamera::grabFrame(cv::Mat& outFrame, int timeoutMs) {
    if (m_handle == nullptr || !m_isStreaming) return false;

    MV_FRAME_OUT stImageInfo = { 0 };

    // 主动向底层缓存索要一张图片
    int nRet = MV_CC_GetImageBuffer(m_handle, &stImageInfo, timeoutMs);

    if (nRet == MV_OK) {
        // 将海康的数据转换为 OpenCV 的 Mat 格式
        // 注意：这里假设相机输出的是 Mono8 (灰度) 或 Bayer 格式，如果是彩色可能需要额外转换。
        // 为了通用性，我们将原始数据直接拷贝进 Mat

        cv::Mat temp(stImageInfo.stFrameInfo.nHeight,
            stImageInfo.stFrameInfo.nWidth,
            CV_8UC1, // 假设 8位单通道，实际开发中需根据 stImageInfo.stFrameInfo.enPixelType 判断
            stImageInfo.pBufAddr);

        // 深拷贝，因为马上要释放海康的内存
        temp.copyTo(outFrame);

        // 极其重要：获取完之后必须释放海康内部的 Buffer，否则会导致内存耗尽
        MV_CC_FreeImageBuffer(m_handle, &stImageInfo);

        return true;
    }

    return false;
}

// ---------------------------------------------------------
// 参数控制实现 (略作演示，详细调用查阅海康手册)
// ---------------------------------------------------------
CameraStatus HikCamera::setExposureTime(float timeUs) {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;
    int nRet = MV_CC_SetFloatValue(m_handle, "ExposureTime", timeUs);
    return (nRet == MV_OK) ? CameraStatus::SUCCESS : CameraStatus::PARAM_SET_FAILED;
}

float HikCamera::getExposureTime() {
    if (m_handle == nullptr) return 0.0f;
    MVCC_FLOATVALUE stParam;
    MV_CC_GetFloatValue(m_handle, "ExposureTime", &stParam);
    return stParam.fCurValue;
}

CameraStatus HikCamera::setGain(float gain) {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;
    int nRet = MV_CC_SetFloatValue(m_handle, "Gain", gain);
    return (nRet == MV_OK) ? CameraStatus::SUCCESS : CameraStatus::PARAM_SET_FAILED;
}

float HikCamera::getGain() {
    if (m_handle == nullptr) return 0.0f;
    MVCC_FLOATVALUE stParam;
    MV_CC_GetFloatValue(m_handle, "Gain", &stParam);
    return stParam.fCurValue;
}