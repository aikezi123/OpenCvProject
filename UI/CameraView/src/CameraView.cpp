// UI/CameraView/src/CameraView.cpp
#include "CameraView.h"
#include <QImage>
#include <QPixmap>

CameraView::CameraView(QWidget* parent) : QWidget(parent) {
    // 1. 初始化底板控件
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);

    // 给视频区加一个纯黑色的背景，这样在没有图像时看起来像个真实的显示屏
    m_videoLabel->setStyleSheet("background-color: black;");

    // 2. 将 Label 填满整个自定义 Widget
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_videoLabel);
    layout->setContentsMargins(0, 0, 0, 0); // 消除边缘留白
    setLayout(layout);
}

CameraView::~CameraView() {}

void CameraView::onFrameReady(const cv::Mat& frame) {
    if (frame.empty()) return;

    QImage qImg;
    cv::Mat rgbMat; // 提上来作为局部变量，确保生命周期

    if (frame.channels() == 1) {
        qImg = QImage((const unsigned char*)(frame.data),
            frame.cols, frame.rows, frame.step,
            QImage::Format_Grayscale8);
    }
    else if (frame.channels() == 3) {
        // 【核心修复】：彻底消灭阿凡达！
        // 强制把 OpenCV 的 BGR 转换为 Qt 喜欢的 RGB
        cv::cvtColor(frame, rgbMat, cv::COLOR_BGR2RGB);

        // 告诉 Qt 这是标准的 RGB888 格式
        qImg = QImage((const unsigned char*)(rgbMat.data),
            rgbMat.cols, rgbMat.rows, rgbMat.step,
            QImage::Format_RGB888);
    }

    if (qImg.isNull()) return;

    // 极速缩放并渲染上屏
    // 注意：qImg.scaled() 会进行深拷贝，所以即使局部变量 rgbMat 随后销毁也是安全的
    QImage scaledImg = qImg.scaled(m_videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::FastTransformation);

    m_videoLabel->setPixmap(QPixmap::fromImage(scaledImg));
}

void CameraView::showMessage(const QString& msg) {
    // 如果没有画面，就在屏幕正中央显示红色的报警文字
    m_videoLabel->clear();
    m_videoLabel->setText(msg);
    m_videoLabel->setStyleSheet("background-color: black; color: #ff4d4f; font-size: 24px; font-weight: bold;");
}