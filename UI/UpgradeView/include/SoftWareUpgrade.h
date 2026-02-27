#pragma once

#include <QWidget>
#include "ui_SoftWareUpgrade.h"
#include <QLabel>
#include <QVBoxLayout>

class SoftWareUpgrade : public QWidget
{
	Q_OBJECT

public:
	SoftWareUpgrade(QWidget *parent = nullptr);
	~SoftWareUpgrade();

private:
	Ui::SoftWareUpgradeClass ui;
};

