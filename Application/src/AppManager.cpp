// Application/src/AppManager.cpp
#include "AppManager.h"
#include "HikCamera.h" // 只有实现文件需要知道具体的底层是谁

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
    // 线程启动 -> Service 开始抓图
    connect(m_cameraThread, &QThread::started, 
            m_cameraService, &CameraService::startWorkLoop);

    // Service 抓到图 -> 发送给 MainWindow 内部的 CameraView 进行绘制
    connect(m_cameraService, &CameraService::frameReadyToShow, 
            m_mainWindow->getCameraView(), &CameraView::onFrameReady);

    // 线程结束时自动清理
    connect(m_cameraThread, &QThread::finished, m_cameraThread, &QObject::deleteLater);
}

void AppManager::start() {
    CameraStatus status = m_camera->openDevice();

    if (status == CameraStatus::SUCCESS) {
        qDebug() << "[AppManager] 相机打开成功，启动数据流线程！";
        m_cameraThread->start();
    }
    else {
        qDebug() << "[AppManager] 致命错误：相机打开失败！错误码：" << static_cast<int>(status);
        // 直接在黑屏上打出提示语！
        m_mainWindow->getCameraView()->showMessage("相机连接失败，请检查网线、电源或 IP 配置！");
    }

    m_mainWindow->show();
}