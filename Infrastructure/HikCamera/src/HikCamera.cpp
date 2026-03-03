// Infrastructure/HikCamera/src/HikCamera.cpp

#include "HikCamera.h"
#include "MvCameraControl.h" // 只有实现文件才认识海康 SDK
#include <iostream>

// ====================================================
// 全局自由函数：作为海康 SDK 的 C 语言风格回调接收器
// ====================================================
static void __stdcall GlobalImageCallback(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    if (pUser) {
        // 1. 将 void* 上下文强转回相机类实例
        HikCamera* pCamera = static_cast<HikCamera*>(pUser);

        // 2. 将海康数据转换为 OpenCV Mat 格式 (这里假设是 8位单通道灰度图)
        cv::Mat tempMat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC1, pData);

        // 3. 触发类内部的回调触发器 (必须 clone 深拷贝！)
        pCamera->triggerCallback(tempMat.clone());
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
    // ... 这里写你之前的枚举逻辑 ...
    return cameraList;
}

CameraStatus HikCamera::openDevice(const std::string& serialNumber) {
    // ... 这里写你之前的打开逻辑 ...
    return CameraStatus::SUCCESS; // 示例
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