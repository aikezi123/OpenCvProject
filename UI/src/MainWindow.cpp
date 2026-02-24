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

    displayImage();
    signalSlotInit();


    connect(m_worker, &EyeWorker::errorOccurred, this, [this](const QString& msg) {
        QMessageBox::warning(this, "错误", msg);
        });
    connect(this, &MainWindow::requestProcessImage, m_worker, &EyeWorker::processImageFromFile);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_workerThread->start();


}
// ... 析构函数保持不变 ...
MainWindow::~MainWindow() {
    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    delete ui;
}

void MainWindow::signalSlotInit() {
    // 在 MainWindow.cpp 的构造函数里修改：
    connect(m_worker, &EyeWorker::frameProcessed, this, [this](const QImage& img) {
        m_currentImage = img; // 1. 先把原图死死攥在手里保存下来
        updateImageDisplay(); // 2. 调用上面的函数去缩放并显示
        });

    // 3. 按钮点击
    connect(ui->btnStart, &QPushButton::clicked, this, [this]() {
        // 1. 获取当前 AppUI.exe 所在的绝对路径 (也就是 .../out/build/debug/bin)
        QString exePath = QCoreApplication::applicationDirPath();

        // 2. 核心魔法：向上退 4 层回到工程根目录，再进入 images 文件夹
        // 路径拼接：exePath + "/../../../../images"
        // QDir().absolutePath() 会自动帮你把这些 "../" 算明白，变成一个干净的绝对路径
        QString targetDirPath = QDir(exePath + "/../../../../images").absolutePath();

        QString filePath = QFileDialog::getOpenFileName(this, "选择眼球图片", targetDirPath, "图片文件 (*.png *.jpg *.bmp)");
        if (!filePath.isEmpty()) emit requestProcessImage(filePath);
        });
}

void MainWindow::displayImage() {
    // 在 MainWindow.cpp 的构造函数或 initUI 函数中，先设置 Label 居中显示
    ui->lblVideo->setAlignment(Qt::AlignCenter);
    // （可选）给 Label 加个黑色的背景，这样留白的地方就是黑边，看起来像电影播放器
    ui->lblVideo->setStyleSheet("QLabel { background-color : black; }");
    ui->lblVideo->setMinimumSize(1, 1);
}

// 专门负责把保存的原图，按照当前 label 的大小重新缩放并显示
void MainWindow::updateImageDisplay() {
    // 只有当图片存在时才处理（防止刚打开软件还没点开始时崩溃）
    if (!m_currentImage.isNull()) {
        ui->lblVideo->setPixmap(QPixmap::fromImage(m_currentImage).scaled(
            ui->lblVideo->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        ));
    }
}

// 窗口拉伸事件监听器
void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event); // 极其重要：必须先调回父类的默认处理
    updateImageDisplay();            // 然后趁机重新缩放我们的图片！
}