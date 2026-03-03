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
    // 优雅退出机制：严格按照创建的逆序进行销毁
    if (m_cameraThread) {
        m_cameraThread->quit();
        m_cameraThread->wait();
    }
    
    // Qt 的对象树或智能指针会自动处理一些，但手动管理裸指针更显式安全
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
    // 启动顺序：先开硬件，再开线程，最后展示 UI
    if (m_camera->openDevice() == CameraStatus::SUCCESS) {
        m_cameraThread->start();
    }
    
    m_mainWindow->show();
}