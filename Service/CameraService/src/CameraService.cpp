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