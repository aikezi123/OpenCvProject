// Application/include/AppManager.h
#pragma once

#include <QObject>
#include <QThread>

// 引入各层的头文件
#include "MainWindow.h"
#include "ICamera.h"
#include "CameraService.h"

class AppManager : public QObject {
    Q_OBJECT
public:
    explicit AppManager(QObject* parent = nullptr);
    ~AppManager() override;

    // 统一步骤：初始化 -> 装配 -> 启动
    void initialize();
    void start();

private:
    // 专门用来处理跨层信号与槽的连线
    void wireConnections();

private:
    // ==========================================
    // 系统的核心组件 (未来这里可以加激光器、数据库等)
    // ==========================================
    
    // UI 层
    MainWindow* m_mainWindow;

    // 基础设施与业务层
    ICamera* m_camera;

    // 服务层与线程
    CameraService* m_cameraService;
    QThread* m_cameraThread;
};