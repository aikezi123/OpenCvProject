#include "AppManager.h"
#include "HikCamera.h" 
#include <QDebug>

AppManager::AppManager(QObject* parent)
    : QObject(parent),
    m_mainWindow(nullptr),
    m_camera(nullptr),
    m_cameraService(nullptr),
    m_cameraThread(nullptr) {
}

AppManager::~AppManager() {
    // 1. 【极其致命的顺序】：必须先让底层硬件彻底停止抓图，掐断幽灵回调的源头！
    if (m_camera) {
        m_camera->stopStream();
        m_camera->closeDevice();
    }

    // 2. 然后再优雅退出 Qt 线程
    if (m_cameraThread) {
        m_cameraThread->quit();
        m_cameraThread->wait();
    }

    // 3. 最后再安全地释放各层指针 (此时绝对不会再有回调来打扰它们了)
    delete m_cameraService;
    delete m_camera;
    delete m_mainWindow;
}

void AppManager::initialize() {
    // 1. 实例化硬件接口
    m_camera = new HikCamera();

    // 2. 实例化服务层并注入硬件
    m_cameraService = new CameraService(m_camera);

    // 3. 实例化 UI
    m_mainWindow = new MainWindow();

    // 4. 配置多线程
    m_cameraThread = new QThread(this);
    m_cameraService->moveToThread(m_cameraThread);

    // 5. 进行信号槽装配
    wireConnections();
}

void AppManager::wireConnections() {
    // ---- A. 线程与生命周期 ----
    connect(m_cameraThread, &QThread::started,
        m_cameraService, &CameraService::startWorkLoop);
    connect(m_cameraThread, &QThread::finished,
        m_cameraThread, &QObject::deleteLater);

    // ---- B. 业务数据流 (Service -> UI) ----
    connect(m_cameraService, &CameraService::frameReadyToShow,
        m_mainWindow->getCameraView(), &CameraView::onFrameReady);

    // ---- C. 控制流 (UI -> Service) 【本次新增】 ----
    // UI 参数调节 -> Service 参数下发 (Service 会通过多态指针调用底层硬件)
    connect(m_mainWindow->getCameraView(), &CameraView::exposureTimeChanged,
        m_cameraService, &CameraService::setExposureTime);

    connect(m_mainWindow->getCameraView(), &CameraView::gainChanged,
        m_cameraService, &CameraService::setGain);
}

void AppManager::start() {
    CameraStatus status = m_camera->openDevice();

    if (status == CameraStatus::SUCCESS) {
        qDebug() << "[AppManager] 相机打开成功，启动数据流线程！";

        // 1. 捞取相机当前值
        float currentExposure = m_camera->getExposureTime();
        float currentGain = m_camera->getGain();

        // 2. 【新增】：捞取相机的物理极限值
        float maxGain = m_camera->getMaxGain();

        // 3. 把这些真实的数据一股脑喂给 UI，让 UI 根据硬件能力变形！
        m_mainWindow->getCameraView()->setInitialParams(currentExposure, currentGain, maxGain);

        m_cameraThread->start();
    }
    else {
        qDebug() << "[AppManager] 致命错误：相机打开失败！错误码：" << static_cast<int>(status);
        m_mainWindow->getCameraView()->showMessage("相机连接失败，请检查网线、电源或 USB 配置！");
    }

    m_mainWindow->showMaximized();
}