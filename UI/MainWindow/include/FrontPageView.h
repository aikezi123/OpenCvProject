#pragma once

#include <QMainWindow>
#include "ui_FrontPageView.h"

class FrontPageView : public QMainWindow
{
	Q_OBJECT

public:
	FrontPageView(QWidget *parent = nullptr);
	~FrontPageView();

	//——————界面样式——————
	void initUiStyle();


private:
	Ui::FrontPageViewClass ui;
};

