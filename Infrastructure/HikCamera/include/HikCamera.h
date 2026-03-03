// Infrastructure/HikCamera/include/HikCamera.h
#pragma once

#include "ICamera.h" // 纯 C++ 契约

class HikCamera : public ICamera {
public:
    HikCamera();
    ~HikCamera() override;

    // ---------------------------------------------------------
    // 强制实现 ICamera 的纯虚函数
    // ---------------------------------------------------------
    std::vector<CameraInfo> enumDevices() override;
    CameraStatus openDevice(const std::string& serialNumber = "") override;
    CameraStatus closeDevice() override;

    CameraStatus startStream() override;
    CameraStatus stopStream() override;

    CameraStatus setExposureTime(float timeUs) override;
    float getExposureTime() override;

    CameraStatus setGain(float gain) override;
    float getGain() override;

    bool grabFrame(cv::Mat& outFrame, int timeoutMs = 1000) override;

    // ---------------------------------------------------------
    // 异步回调机制
    // ---------------------------------------------------------
    void registerFrameCallback(FrameCallback callback) override;

    // 给底层的全局 C 函数用的公开触发接口 (解决 C2039 报错)
    void triggerCallback(const cv::Mat& frame);

private:
    void* m_handle;             // 海康相机句柄
    bool m_isStreaming;         // 取流标志位
    FrameCallback m_callback;   // 保存上层传进来的业务逻辑
};