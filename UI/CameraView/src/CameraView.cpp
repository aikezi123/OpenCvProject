// UI/CameraView/src/CameraView.cpp
#include "CameraView.h"
#include <QImage>
#include <QPixmap>
#include <QDir>


QString btnStyle = "QPushButton { padding: 10px 20px; font-weight: bold; font-size: 14px; background-color: #00b8cc; color: white; border-radius: 4px; }"
"QPushButton:hover { background-color: #00d2e6; }";
QString activeBtnStyle = "QPushButton { padding: 10px 20px; font-weight: bold; font-size: 14px; background-color: #ff9800; color: white; border-radius: 4px; }";

// 构造函数初始化列表中，默认关闭实时流，关闭单帧捕获
CameraView::CameraView(QWidget* parent) : QWidget(parent), m_isLiveMode(false), m_captureNextFrame(false) {
    initSettingsPanel(); // 先初始化子面板
    initLayout();        // 再拼装整体布局
    initSignalConnects();// 最后绑定所有信号槽

    m_btnLiveStream->hide();

}
void CameraView::initLayout() {
    // 1. 视频主显示区 (默认纯黑)
    m_videoLabel = new QLabel(this);
    m_videoLabel->setAlignment(Qt::AlignCenter);
    m_videoLabel->setStyleSheet("background-color: black;");

    // 2. 底部控制栏
    m_btnLiveStream = new QPushButton("获取视频流", this);
    m_btnSnapshot = new QPushButton("捕获单帧", this);
    m_btnSettings = new QPushButton("参数设置", this); // 新增

    m_btnLiveStream->setStyleSheet(btnStyle);
    m_btnSnapshot->setStyleSheet(btnStyle);
    m_btnSettings->setStyleSheet(btnStyle);

    QHBoxLayout* controlLayout = new QHBoxLayout();
    controlLayout->addStretch();
    controlLayout->addWidget(m_btnLiveStream);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(m_btnSnapshot);
    controlLayout->addSpacing(20);
    controlLayout->addWidget(m_btnSettings); // 加入控制栏
    controlLayout->addStretch();

    // 3. 整体横向切割 (左侧视频 + 右侧面板)
    QHBoxLayout* topSplitLayout = new QHBoxLayout();
    topSplitLayout->addWidget(m_videoLabel, 1);    // 占用所有剩余空间
    topSplitLayout->addWidget(m_settingsPanel, 0); // 占用固定空间

    // 4. 最终主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topSplitLayout, 1);
    mainLayout->addLayout(controlLayout);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    setLayout(mainLayout);
}
// ==========================================
// 集中管理所有的信号与槽交互
// ==========================================
void CameraView::initSignalConnects() {
    // 1. 获取视频流按钮
    connect(m_btnLiveStream, &QPushButton::clicked, this, [this]() {
        m_isLiveMode = true;        // 开启常开阀门
        m_captureNextFrame = false; // 取消单帧特权
        m_btnLiveStream->setStyleSheet(activeBtnStyle);
        m_btnSnapshot->setStyleSheet(btnStyle);
        });

    // 2. 捕获单帧按钮
    connect(m_btnSnapshot, &QPushButton::clicked, this, [this]() {
        m_isLiveMode = false;       // 关掉常开阀门，让视频流停下
        m_captureNextFrame = true;  // 赋予下一帧特权

        m_btnSnapshot->setStyleSheet(activeBtnStyle);
        m_btnLiveStream->setStyleSheet(btnStyle);
        });

    // 3. 侧边栏抽屉开关按钮
    connect(m_btnSettings, &QPushButton::clicked, this, [this]() {
        if (m_settingsPanel->isVisible()) {
            m_settingsPanel->hide();
            m_btnSettings->setStyleSheet(btnStyle);
        }
        else {
            m_settingsPanel->show();
            m_btnSettings->setStyleSheet(activeBtnStyle);
        }
        });

    // 4. 数值变化时，向外发射信号
    connect(m_spinExposure, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        emit exposureTimeChanged(val);
        });

    connect(m_spinGain, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        emit gainChanged(val);
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

        // 【配合调用】：传入当前帧，以及你想要的相对或绝对路径
        // 这里以程序运行目录下的 "CaptureData/Images" 文件夹为例
        saveFrameToFile(frame, "./CaptureData/Images");
    }
}

void CameraView::showMessage(const QString& msg) {
    // 如果没有画面，就在屏幕正中央显示红色的报警文字
    m_videoLabel->clear();
    m_videoLabel->setText(msg);
    m_videoLabel->setStyleSheet("background-color: black; color: #ff4d4f; font-size: 24px; font-weight: bold;");
}

// ==========================================
// 软硬件同步：设置界面的初始真实值
// ==========================================
void CameraView::setInitialParams(double exposure, double gain, double maxGain) {
    // 曝光时间的同步
    m_spinExposure->blockSignals(true);
    m_spinExposure->setValue(exposure);
    m_spinExposure->blockSignals(false);

    // 增益的同步与【动态边界重置】
    m_spinGain->blockSignals(true);
    // 核心代码：把刚才写死的 12.0265 换成底层传来的真实极限！
    m_spinGain->setRange(0.0, maxGain);
    m_spinGain->setValue(gain);
    m_spinGain->blockSignals(false);

    qDebug() << "[CameraView] UI 已同步相机真实参数 -> 曝光:" << exposure
        << "us, 当前增益:" << gain << "dB, 增益极限被锁定为:" << maxGain << "dB";
}

void CameraView::saveFrameToFile(const cv::Mat& frame, const QString& dirPath) {
    if (frame.empty()) return;

    // 1. 安全检查：如果目标文件夹不存在，则自动创建！
    QDir dir;
    if (!dir.exists(dirPath)) {
        dir.mkpath(dirPath);
    }

    // 2. 生成绝对路径和带毫秒时间戳的文件名
    QString fileName = dirPath + "/IMG_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz") + ".jpg";

    // 3. 格式安全转换：
    // 因为传进来的 frame 已经被底层转成了 Qt 喜欢的 RGB，
    // 但是 cv::imwrite 骨子里只认 BGR！如果不反转一下，存下来的图片就会变成蓝脸“阿凡达”。
    if (frame.channels() == 3) {
        cv::Mat bgrMat;
        cv::cvtColor(frame, bgrMat, cv::COLOR_RGB2BGR);
        cv::imwrite(fileName.toStdString(), bgrMat);
    }
    else {
        // 单通道黑白图直接存
        cv::imwrite(fileName.toStdString(), frame);
    }
}


// ==========================================
// 专门负责构建右侧参数面板
// ==========================================
void CameraView::initSettingsPanel() {
    m_settingsPanel = new QWidget(this);
    m_settingsPanel->setFixedWidth(250);
    m_settingsPanel->setStyleSheet("background-color: #f0f0f0; border-left: 1px solid #ccc;");
    m_settingsPanel->hide(); // 默认收起状态

    QFormLayout* formLayout = new QFormLayout(m_settingsPanel);
    formLayout->setContentsMargins(15, 20, 15, 20);
    formLayout->setSpacing(15);

    QLabel* panelTitle = new QLabel("相机参数设置");
    panelTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #333; border: none;");
    formLayout->addRow(panelTitle);

    // 曝光时间配置
    m_spinExposure = new QDoubleSpinBox(m_settingsPanel);
    m_spinExposure->setRange(100.0, 100000.0);
    m_spinExposure->setValue(10000.0);
    m_spinExposure->setSingleStep(1000.0);
    m_spinExposure->setSuffix(" us");

    // 增益配置
    m_spinGain = new QDoubleSpinBox(m_settingsPanel);
    m_spinGain->setDecimals(4);             // 【新增】：保留4位小数，精确匹配海康的硬件精度
    m_spinGain->setRange(0.0, 12.0265);     // 【修改】：严格限制上限为 12.0265
    m_spinGain->setValue(0.0);
    m_spinGain->setSingleStep(0.5);         // 【修改】：步长改为 0.5，方便精细调节
    m_spinGain->setSuffix(" dB");           // 【新增】：加上专业单位后缀

    formLayout->addRow("曝光时间:", m_spinExposure);
    formLayout->addRow("模拟增益:", m_spinGain);
}