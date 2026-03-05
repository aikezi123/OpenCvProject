// Service/CameraService/src/CameraService.cpp
#include "CameraService.h"
#include <QThread>
#include <QDebug>

CameraService::CameraService(ICamera* camera, QObject* parent)
    : QObject(parent), m_camera(camera), m_isWorking(false) {
}

CameraService::~CameraService() {
    stopWorkLoop();
}

void CameraService::startWorkLoop() {
    if (!m_camera) return;

    // ====================================================
    // 依赖注入的核心：把带有 Qt 信号的 Lambda 塞给纯 C++ 的底层！
    // ====================================================
    m_camera->registerFrameCallback([this](const cv::Mat& frame) {
        // 这个 Lambda 实际是在海康的底层线程中被触发的
        // 但放心，Qt 的 emit 非常聪明，它发现跨线程时，会自动变成队列投递 (QueuedConnection)
        // 绝对不会卡死或者搞崩 UI 线程！
        emit frameReadyToShow(frame);
        });

    // 告诉底层：开始取流吧，有图了就调我上面那个 Lambda
    m_camera->startStream();

    emit serviceMessage("Camera async callback registered and stream started.");
}

void CameraService::stopWorkLoop() {
    m_isWorking = false;
}

// ==========================================
// 接收 UI 指令，并转发给底层硬件执行
// ==========================================
void CameraService::setExposureTime(double timeUs) {
    if (m_camera) {
        // 强转为 float，因为 ICamera 接口定义的是 float
        m_camera->setExposureTime(static_cast<float>(timeUs));
        qDebug() << "[CameraService] 已命令硬件修改曝光时间:" << timeUs << "us";
    }
}

void CameraService::setGain(double gain) {
    if (m_camera) {
        m_camera->setGain(static_cast<float>(gain));
        qDebug() << "[CameraService] 已命令硬件修改增益:" << gain;
    }
}