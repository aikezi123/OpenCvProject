// Service/CameraService/include/CameraService.h
#pragma once

#include <QObject>
#include <atomic>
#include <opencv2/opencv.hpp>
#include "ICamera.h" // 认识业务契约

class CameraService : public QObject {
    Q_OBJECT // 开启信号槽能力
public:
    // 【依赖注入】：服务层不自己去 new 相机，而是让外部（Application）传进来
    explicit CameraService(ICamera* camera, QObject* parent = nullptr);
    ~CameraService() override;

public slots:
    // 启动业务主循环（这个函数未来会在 Application 分配的子线程中运行）
    void startWorkLoop();

    // 停止业务主循环
    void stopWorkLoop();

    // 接收 UI 传来的参数设置指令
    void setExposureTime(double timeUs);
    void setGain(double gain);


signals:
    // 【跨界翻译】：当底层抓到纯 C++ 的 cv::Mat 图像后，通过 Qt 信号发给 UI 层
    void frameReadyToShow(const cv::Mat& image);

    // 向外部报告服务状态或错误（可选）
    void serviceMessage(const QString& msg);

private:
    ICamera* m_camera;                // 底层相机实例指针
    std::atomic<bool> m_isWorking;    // 线程安全的循环标志位
};