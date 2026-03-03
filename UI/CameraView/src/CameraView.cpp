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
    // 针对海康工业相机的常见格式进行转换
    if (frame.channels() == 1) {
        // 8位单通道灰度图
        qImg = QImage((const unsigned char*)(frame.data),
            frame.cols, frame.rows, frame.step,
            QImage::Format_Grayscale8);
    }
    else if (frame.channels() == 3) {
        // 8位3通道彩色图 (OpenCV 默认 BGR，Qt 是 RGB)
        cv::Mat rgbMat;
        cv::cvtColor(frame, rgbMat, cv::COLOR_BGR2RGB);
        qImg = QImage((const unsigned char*)(rgbMat.data),
            rgbMat.cols, rgbMat.rows, rgbMat.step,
            QImage::Format_RGB888);
    }

    // 格式转换后，平滑缩放以适应当前窗口大小，并绘制上去 (注意 copy() 防止内存撕裂)
    m_videoLabel->setPixmap(QPixmap::fromImage(qImg.copy()).scaled(
        m_videoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}