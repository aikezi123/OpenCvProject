#pragma once

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <functional>

// =========================================================
// 相机状态枚举：规范化错误处理，避免只返回 true/false
// =========================================================
enum class CameraStatus {
    SUCCESS,            // 操作成功
    DEVICE_NOT_FOUND,   // 未找到设备
    OPEN_FAILED,        // 打开设备失败（可能被占用）
    STREAM_FAILED,      // 开启/关闭取流失败
    PARAM_SET_FAILED,   // 参数设置或获取失败
    DISCONNECTED,       // 设备意外断开连接
    UNKNOWN_ERROR       // 未知错误
};

// =========================================================
// 相机设备信息：用于在界面下拉框中展示可用的相机
// =========================================================
struct CameraInfo {
    std::string serialNumber; // 唯一序列号 (如海康的 "00E123456")
    std::string modelName;    // 设备型号 (如 "MV-CA013-20GC")
    std::string ipAddress;    // IP地址 (网口相机专用)

    // 方便测试打印
    std::string toString() const {
        return modelName + " (" + serialNumber + ")";
    }
};

// 定义一个标准的纯 C++ 回调函数签名
// 意思是：我需要一个函数，它接收一个 cv::Mat，并且没有返回值
using FrameCallback = std::function<void(const cv::Mat&)>;

// =========================================================
// ICamera 核心设备接口 (Business 层契约)
// 作用：隔离底层硬件，定义“一个标准的相机应该能做什么”
// =========================================================
class ICamera {
public:
    // 【核心法则】：多态基类必须有虚析构函数！
    // 确保 Application 层 delete 接口指针时，能正确调用底层实现类（如 HikCamera）的析构函数。
    virtual ~ICamera() = default;

    // ---------------------------------------------------------
    // 1. 设备发现与生命周期管理
    // ---------------------------------------------------------

    /**
     * @brief 枚举当前系统中所有连接的该类型相机
     * @return 包含相机信息的列表
     */
    virtual std::vector<CameraInfo> enumDevices() = 0;

    /**
     * @brief 打开指定的相机
     * @param serialNumber 相机序列号。如果为空字符串，则默认打开枚举到的第一台相机
     * @return 操作状态
     */
    virtual CameraStatus openDevice(const std::string& serialNumber = "") = 0;

    /**
     * @brief 关闭设备并释放底层硬件句柄
     */
    virtual CameraStatus closeDevice() = 0;

    // ---------------------------------------------------------
    // 2. 视频流控制
    // ---------------------------------------------------------
    // 异步回调机制
    /**
     * @brief 注册图像抓取回调函数
     * @param callback 外部传入的函数对象 (Lambda, 类的成员函数等)
     */
    virtual void registerFrameCallback(FrameCallback callback) = 0;
    /**
     * @brief 开始向内存传输图像数据（启动底层的连续抓图机制）
     */
    virtual CameraStatus startStream() = 0;

    /**
     * @brief 停止图像数据传输
     */
    virtual CameraStatus stopStream() = 0;

    // ---------------------------------------------------------
    // 3. 核心参数控制
    // ---------------------------------------------------------

    /**
     * @brief 设置曝光时间
     * @param timeUs 曝光时间，单位：微秒(us)
     */
    virtual CameraStatus setExposureTime(float timeUs) = 0;

    virtual float getExposureTime() = 0;

    /**
     * @brief 设置图像增益
     * @param gain 增益值
     */
    virtual CameraStatus setGain(float gain) = 0;

    virtual float getGain() = 0;

    // ---------------------------------------------------------
    // 4. 同步图像获取 (可选)
    // ---------------------------------------------------------
    /**
     * @brief 主动拉取一帧图像（适用于单线程或特定算法同步等待的场景）
     * @param outFrame [out] 传出的 OpenCV 矩阵数据
     * @param timeoutMs 超时时间(毫秒)
     * @return 是否成功获取
     */
    virtual bool grabFrame(cv::Mat& outFrame, int timeoutMs = 1000) = 0;
};