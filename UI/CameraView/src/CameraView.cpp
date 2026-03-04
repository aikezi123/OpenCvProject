// UI/CameraView/src/CameraView.cpp
#include "CameraView.h"
#include <QImage>
#include <QPixmap>


QString btnStyle = "QPushButton { padding: 10px 20px; font-weight: bold; font-size: 14px; background-color: #00b8cc; color: white; border-radius: 4px; }"
"QPushButton:hover { background-color: #00d2e6; }";
QString activeBtnStyle = "QPushButton { padding: 10px 20px; font-weight: bold; font-size: 14px; background-color: #ff9800; color: white; border-radius: 4px; }";

// 构造函数初始化列表中，默认关闭实时流，关闭单帧捕获
CameraView::CameraView(QWidget* parent) : QWidget(parent), m_isLiveMode(false), m_captureNextFrame(false) {
    initLayout();
    initSignalConnects();

    m_btnLiveStream->hide();

}
void CameraView::initLayout() {
    // 1. 视频主显示区 (默认纯黑)
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("background-color: black;");

    // 2. 底部控制栏
    m_btnLiveStream = new QPushButton("获取视频流 ", this);
    m_btnSnapshot = new QPushButton("捕获单帧 ", this);

    m_btnLiveStream->setStyleSheet(btnStyle);
    m_btnSnapshot->setStyleSheet(btnStyle);

    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->addStretch();
    controlLayout->addWidget(m_btnLiveStream);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(m_btnSnapshot);
    controlLayout->addStretch();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_videoLabel, 1);
    mainLayout->addLayout(controlLayout);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    setLayout(mainLayout);
}
void CameraView::initSignalConnects() {
    // ==========================================
// 4. 核心：精确的状态机切换
// ==========================================
    connect(m_btnLiveStream, &QPushButton::clicked, this, [this]() {
        m_isLiveMode = true;        // 开启常开阀门
        m_captureNextFrame = false; // 取消单帧特权

        m_btnLiveStream->setStyleSheet(activeBtnStyle);
        m_btnSnapshot->setStyleSheet(btnStyle);
        });

    connect(m_btnSnapshot, &QPushButton::clicked, this, [this]() {
        m_isLiveMode = false;       // 关掉常开阀门，让视频流停下
        m_captureNextFrame = true;  // 【精髓】：赋予下一帧特权，让它流进 UI！

        m_btnSnapshot->setStyleSheet(activeBtnStyle);
        m_btnLiveStream->setStyleSheet(btnStyle);
        });
}


CameraView::~CameraView() {}

// ==========================================
// 5. 渲染拦截与画面定格控制
// ==========================================
void CameraView::onFrameReady(const cv::Mat& frame) {
    if (frame.empty()) return;

    // 【核心控制阀门】：如果既不是“实时视频模式”，也没有获得“单帧放行特权”，直接丢弃画面！
    if (!m_isLiveMode && !m_captureNextFrame) return;

    // ---- 图像格式转换 (OpenCV Mat -> Qt QImage) ----
    QImage qImg;
    if (frame.channels() == 1) {
        qImg = QImage((const unsigned char*)(frame.data),
            frame.cols, frame.rows, frame.step,
            QImage::Format_Grayscale8);
    }
    else if (frame.channels() == 3) {
        qImg = QImage((const unsigned char*)(frame.data),
            frame.cols, frame.rows, frame.step,
            QImage::Format_RGB888);
    }

    if (qImg.isNull()) return;

    // 极速缩放并渲染到界面上
    QImage scaledImg = qImg.scaled(m_videoLabel->size(),
        Qt::KeepAspectRatio,
        Qt::FastTransformation);
    m_videoLabel->setPixmap(QPixmap::fromImage(scaledImg));

    // ---- 拦截后续操作 ----
    // 如果刚才使用的是“单帧放行特权”，现在图已经画到屏幕上了，特权必须立刻收回！
    // 这样下一帧就会被上面的 if 拦截掉，画面完美定格。
    if (m_captureNextFrame) {
        m_captureNextFrame = false;
    }
}

void CameraView::showMessage(const QString& msg) {
    // 如果没有画面，就在屏幕正中央显示红色的报警文字
    m_videoLabel->clear();
    m_videoLabel->setText(msg);
    m_videoLabel->setStyleSheet("background-color: black; color: #ff4d4f; font-size: 24px; font-weight: bold;");
}