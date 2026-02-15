#pragma once

#include <QMainWindow>
#include "ui_ImageShowView.h"

class ImageShowView : public QMainWindow
{
	Q_OBJECT

public:
	ImageShowView(QWidget *parent = nullptr);
	~ImageShowView();

private:
	Ui::ImageShowViewClass ui;
};

