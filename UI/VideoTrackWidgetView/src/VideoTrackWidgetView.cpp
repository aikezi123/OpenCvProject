#include "VideoTrackWidgetView.h"

VideoTrackWidgetView::VideoTrackWidgetView(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* label = new QLabel("这里是【实时视频流追踪】界面\n\n即将引入 OpenCV VideoCapture\n开启 30FPS 极速识别挑战！", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("background-color: #2b2b2b; color: #42b983; font-size: 24px; font-weight: bold;");

    layout->addWidget(label);
}