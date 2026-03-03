// Infrastructure/HikCamera/include/HikCamera.h
#pragma once

#include "ICamera.h" // 来自 Business/CameraCore

// 海康的头文件不要写在这里！保持接口层绝对干净。

class HikCamera : public ICamera {
public:
    HikCamera();
    ~HikCamera() override;

    // ---------------------------------------------------------
    // 实现 ICamera 的纯虚函数 (履约)
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

    // 同步抓图接口
    bool grabFrame(cv::Mat& outFrame, int timeoutMs = 1000) override;

private:
    void* m_handle;         // 海康相机的底层句柄
    bool m_isStreaming;     // 是否正在取流
};