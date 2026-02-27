#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "ui_FirmWareUpgradeView.h"

class FirmWareUpgradeView : public QWidget
{
	Q_OBJECT

public:
	FirmWareUpgradeView(QWidget *parent = nullptr);
	~FirmWareUpgradeView();

private:
	Ui::FirmWareUpgradeView ui;
};

