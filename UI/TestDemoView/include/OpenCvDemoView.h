#pragma once

#include <QMainWindow>
#include "ui_OpenCvDemoView.h"
#include <opencv2/opencv.hpp>
#include <QTimer>

class OpenCvDemoView : public QMainWindow
{
	Q_OBJECT

public:
	OpenCvDemoView(QWidget* parent = nullptr);
	~OpenCvDemoView();

	void startCamera();
	void displayMatOnLabel(cv::Mat mat, QLabel* label);

private slots:
	void readFrame();

	// 【新增】各个属性对应的滑块响应槽函数
	void onBrightnessChanged(int value);
	void onExposureChanged(int value);
	void onContrastChanged(int value);

private:
	// 【新增】专门用于初始化 UI 控件与相机属性绑定的函数
	void initCameraControls();

	Ui::OpenCvDemoViewClass ui;
	cv::VideoCapture m_video;
	QTimer* m_timer;
};