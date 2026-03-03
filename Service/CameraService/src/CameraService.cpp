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
    if (!m_camera) {
        emit serviceMessage("Error: Camera instance is null!");
        return;
    }

    m_isWorking = true;
    cv::Mat frame;

    emit serviceMessage("Camera service worker loop started.");

    // ====================================================
    // 核心流水线：这是一个死循环，由于它会被 moveToThread 扔进子线程，
    // 所以这里的死循环绝对不会卡死主界面的 UI！
    // ====================================================
    while (m_isWorking) {
        // 1. 同步向底层硬件索要一帧图像 (超时时间 1000ms)
        bool success = m_camera->grabFrame(frame, 1000);

        if (success && !frame.empty()) {

            // 2. (预留位置)：未来你可以在这里加上 OpenCV 的图像滤波、特征识别等算法
            // cv::Mat processedFrame = doSomeVisionAlgorithm(frame);

            // 3. 将图像发射出去！
            // UI 层只需要连接这个信号，就能在界面上画图了。
            emit frameReadyToShow(frame);

        }
        else {
            // 如果超时没抓到图，可以在这里记录日志，循环会继续重试
            // qDebug() << "Grab frame timeout or failed.";
        }
    }

    emit serviceMessage("Camera service worker loop stopped.");
}

void CameraService::stopWorkLoop() {
    m_isWorking = false;
}