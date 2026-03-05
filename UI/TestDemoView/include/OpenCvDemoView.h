#pragma once

#include <QMainWindow>
#include "ui_OpenCvDemoView.h"
#include <opencv2/opencv.hpp>

class OpenCvDemoView : public QMainWindow
{
	Q_OBJECT

public:
	OpenCvDemoView(QWidget *parent = nullptr);
	~OpenCvDemoView();

	void CvMatCreat();

	void displayMatOnLabel(cv::Mat mat, QLabel* label);

private:
	Ui::OpenCvDemoViewClass ui;
};

