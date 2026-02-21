#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton> 
#include "EyeWorker.h"

// UI 现在只需要引入 EyeWorker，其他的都不需要！
#include "EyeWorker.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindowClass), m_workerThread(new QThread(this)) {
    ui->setupUi(this);

    // 1. 直接创建 Worker (内部已经自己组装好了业务和工具)
    m_worker = new EyeWorker();
    m_worker->moveToThread(m_workerThread);

    // 2. 连线 (保持不变)
    connect(m_worker, &EyeWorker::frameProcessed, this, [this](const QImage& img) {
        ui->lblVideo->setPixmap(QPixmap::fromImage(img).scaled(ui->lblVideo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        });
    connect(m_worker, &EyeWorker::errorOccurred, this, [this](const QString& msg) {
        QMessageBox::warning(this, "错误", msg);
        });
    connect(this, &MainWindow::requestProcessImage, m_worker, &EyeWorker::processImageFromFile);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread->start();

    // 3. 按钮点击
    connect(ui->btnStart, &QPushButton::clicked, this, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, "选择眼球图片", "", "图片文件 (*.png *.jpg *.bmp)");
        if (!filePath.isEmpty()) emit requestProcessImage(filePath);
        });
}
// ... 析构函数保持不变 ...
MainWindow::~MainWindow() {
    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    delete ui;
}