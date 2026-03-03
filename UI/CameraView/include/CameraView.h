// UI/CameraView/include/CameraView.h
#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>

class CameraView : public QWidget {
    Q_OBJECT // 必备：启用信号槽机制
public:
    explicit CameraView(QWidget* parent = nullptr);
    ~CameraView() override;

public slots:
    // 专门用来接收 Service 层发来的图像
    void onFrameReady(const cv::Mat& frame);

private:
    QLabel* m_videoLabel; // 真正用来显示画面的底层控件
};