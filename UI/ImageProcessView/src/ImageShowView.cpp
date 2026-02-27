#include "ImageShowView.h"
#include "ui_ImageShowView.h" // 自动生成的 UI 头文件
#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>

ImageShowView::ImageShowView(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ImageShowView)
    , m_workerThread(new QThread(this))
{
    ui->setupUi(this);

    // UI 初始化配置
    ui->lblVideo->setAlignment(Qt::AlignCenter);
    ui->lblVideo->setStyleSheet("QLabel { background-color : black; color: white; }");
    ui->lblVideo->setMinimumSize(1, 1);

    // 多线程装配
    m_worker = new EyeWorker();
    m_worker->moveToThread(m_workerThread);

    signalSlotInit();
    m_workerThread->start();
}

ImageShowView::~ImageShowView() {
    if (m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
    delete ui;
}

void ImageShowView::signalSlotInit() {
    connect(m_worker, &EyeWorker::errorOccurred, this, [this](const QString& msg) {
        QMessageBox::warning(this, "错误", msg);
        });

    connect(this, &ImageShowView::requestProcessImage, m_worker, &EyeWorker::processImageFromFile);
    connect(m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);

    connect(m_worker, &EyeWorker::frameProcessed, this, [this](const QImage& img) {
        m_currentImage = img;
        updateImageDisplay();
        });

    connect(ui->btnStart, &QPushButton::clicked, this, [this]() {
        QString exePath = QCoreApplication::applicationDirPath();
        QString targetDirPath = QDir(exePath + "/../../../../images").absolutePath();
        QString filePath = QFileDialog::getOpenFileName(this, "选择眼球图片", targetDirPath, "图片文件 (*.png *.jpg *.bmp)");
        if (!filePath.isEmpty()) emit requestProcessImage(filePath);
        });
}

void ImageShowView::updateImageDisplay() {
    if (!m_currentImage.isNull()) {
        ui->lblVideo->setPixmap(QPixmap::fromImage(m_currentImage).scaled(
            ui->lblVideo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
        ));
    }
}

void ImageShowView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateImageDisplay();
}