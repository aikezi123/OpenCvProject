// UI/CameraView/include/CameraView.h
#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <opencv2/opencv.hpp>
#include <qpushbutton.h>

class CameraView : public QWidget {
    Q_OBJECT // 必备：启用信号槽机制
public:
    explicit CameraView(QWidget* parent = nullptr);
    ~CameraView() override;
    void showMessage(const QString& msg);
    void initLayout();
    void initSignalConnects();

public slots:
    // 专门用来接收 Service 层发来的图像
    void onFrameReady(const cv::Mat& frame);


private:
    //保存图片的辅助函数
    void saveFrameToFile(const cv::Mat& frame, const QString& dirPath);

private:
    QLabel* m_videoLabel; // 真正用来显示画面的底层控件

    // UI 控制按钮
    QPushButton* m_btnLiveStream;
    QPushButton* m_btnSnapshot;

    // 核心状态阀门
    bool m_isLiveMode = true; // true: 实时刷新, false: 画面定格
    bool m_captureNextFrame = false; // 新增：是否只放行“单帧”的特权标志
    cv::Mat m_lastFrame;      // 暂存最后一张画面，用于存图


};