// Infrastructure/HikCamera/src/HikCamera.cpp

#include "HikCamera.h"
#include "MvCameraControl.h" // 只有实现文件才认识海康 SDK
#include <iostream>

// ====================================================
// 全局自由函数：作为海康 SDK 的 C 语言风格回调接收器
// 带智能色彩解码的回调
// ====================================================
static void __stdcall GlobalImageCallback(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    if (pUser) {
        HikCamera* pCamera = static_cast<HikCamera*>(pUser);

        // 1. 将海康数据套上 OpenCV 的壳子 (此时还是原始的单通道数据)
        cv::Mat rawMat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);
        cv::Mat colorMat;

        // 2. 识别像素格式，进行色彩还原 (Bayer 还原为 RGB)
        // 注意：海康不同型号的彩色相机可能输出不同的 Bayer 阵列，这里覆盖了最常见的四种
        switch (pFrameInfo->enPixelType) {
        case PixelType_Gvsp_BayerRG8:
            cv::cvtColor(rawMat, colorMat, cv::COLOR_BayerRG2BGR); // 改为 BGR
            break;
        case PixelType_Gvsp_BayerGB8:
            cv::cvtColor(rawMat, colorMat, cv::COLOR_BayerGB2BGR); // 改为 BGR
            break;
        case PixelType_Gvsp_BayerGR8:
            cv::cvtColor(rawMat, colorMat, cv::COLOR_BayerGR2BGR); // 改为 BGR
            break;
        case PixelType_Gvsp_BayerBG8:
            cv::cvtColor(rawMat, colorMat, cv::COLOR_BayerBG2BGR); // 改为 BGR
            break;
        default:
            // 如果是纯黑白相机 (Mono8)，或者无法识别，就保持灰度图
            colorMat = rawMat.clone();
            break;
        }

        // 3. 将处理好的真彩色图像发送出去！
        pCamera->triggerCallback(colorMat);
    }
}

// ====================================================
// 类成员函数实现
// ====================================================

HikCamera::HikCamera() : m_handle(nullptr), m_isStreaming(false) {
}

HikCamera::~HikCamera() {
    closeDevice();
}

void HikCamera::registerFrameCallback(FrameCallback callback) {
    m_callback = callback;
}

void HikCamera::triggerCallback(const cv::Mat& frame) {
    if (m_callback) {
        m_callback(frame); // 实际调用 Service 层传进来的 Lambda
    }
}

CameraStatus HikCamera::startStream() {
    if (m_handle == nullptr) return CameraStatus::OPEN_FAILED;

    // 【关键】：向海康注册全局静态回调，并把自己的指针 (this) 传过去
    MV_CC_RegisterImageCallBackEx(m_handle, GlobalImageCallback, this);

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

// ---------------------------------------------------------
// 其他基础函数的骨架 (防止抽象类报错，你可以补充细节)
// ---------------------------------------------------------

std::vector<CameraInfo> HikCamera::enumDevices() {
    std::vector<CameraInfo> cameraList;
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // 只枚举 USB 接口的相机 (MV_USB_DEVICE)
    int nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList);
    if (nRet != MV_OK) {
        std::cerr << "[HikCamera] 枚举 USB 设备失败! 错误码: " << std::hex << nRet << std::endl;
        return cameraList;
    }

    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        CameraInfo info;

        // 只处理 USB 设备的信息提取
        if (pDeviceInfo->nTLayerType == MV_USB_DEVICE) {
            info.serialNumber = reinterpret_cast<char*>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            info.modelName = reinterpret_cast<char*>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName);
            info.ipAddress = "USB3.0 Direct Connection"; // USB 没有 IP，给个提示占位

            cameraList.push_back(info);
        }
    }

    return cameraList;
}

CameraStatus HikCamera::openDevice(const std::string& serialNumber) {
    if (m_handle != nullptr) {
        return CameraStatus::SUCCESS; // 已经打开了
    }

    MV_CC_DEVICE_INFO_LIST stDeviceList;

    // 【修改点 3】：同样只在 USB 总线上找设备
    int nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &stDeviceList);
    if (nRet != MV_OK || stDeviceList.nDeviceNum == 0) {
        return CameraStatus::DEVICE_NOT_FOUND;
    }

    int targetIndex = -1;

    if (!serialNumber.empty()) {
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
            MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];

            if (pDeviceInfo->nTLayerType == MV_USB_DEVICE) {
                std::string currentSerial = reinterpret_cast<char*>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                if (currentSerial == serialNumber) {
                    targetIndex = i;
                    break;
                }
            }
        }
        if (targetIndex == -1) return CameraStatus::DEVICE_NOT_FOUND;
    }
    else {
        targetIndex = 0; // 默认打开第一台 USB 相机
    }

    // 1. 创建句柄
    nRet = MV_CC_CreateHandle(&m_handle, stDeviceList.pDeviceInfo[targetIndex]);
    if (nRet != MV_OK) return CameraStatus::OPEN_FAILED;

    // 1. 开启连续自动曝光 (ExposureAuto = 2)
    MV_CC_SetEnumValue(m_handle, "ExposureAuto", 2);

    // 2. 开启连续自动白平衡 (BalanceWhiteAuto = 2)
    MV_CC_SetEnumValue(m_handle, "BalanceWhiteAuto", 2);

    // 2. 打开设备
    nRet = MV_CC_OpenDevice(m_handle);
    if (nRet != MV_OK) {
        MV_CC_DestroyHandle(m_handle);
        m_handle = nullptr;
        return CameraStatus::OPEN_FAILED;
    }

    // 3. 强制设置为“连续抓图模式” (关闭触发模式)
    nRet = MV_CC_SetEnumValue(m_handle, "TriggerMode", 0);
    if (nRet != MV_OK) {
        std::cerr << "[HikCamera] 警告：关闭触发模式失败!" << std::endl;
    }

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

bool HikCamera::grabFrame(cv::Mat& outFrame, int timeoutMs) {
    // 虽然有了异步回调，但同步抓图接口也可保留以防万一
    if (m_handle == nullptr || !m_isStreaming) return false;
    MV_FRAME_OUT stImageInfo = { 0 };
    int nRet = MV_CC_GetImageBuffer(m_handle, &stImageInfo, timeoutMs);
    if (nRet == MV_OK) {
        cv::Mat temp(stImageInfo.stFrameInfo.nHeight, stImageInfo.stFrameInfo.nWidth, CV_8UC1, stImageInfo.pBufAddr);
        temp.copyTo(outFrame);
        MV_CC_FreeImageBuffer(m_handle, &stImageInfo);
        return true;
    }
    return false;
}

CameraStatus HikCamera::setExposureTime(float timeUs) {
    if (m_handle) MV_CC_SetFloatValue(m_handle, "ExposureTime", timeUs);
    return CameraStatus::SUCCESS;
}

float HikCamera::getExposureTime() {
    return 0.0f; // 示例
}

CameraStatus HikCamera::setGain(float gain) {
    if (m_handle) MV_CC_SetFloatValue(m_handle, "Gain", gain);
    return CameraStatus::SUCCESS;
}

float HikCamera::getGain() {
    return 0.0f; // 示例
}