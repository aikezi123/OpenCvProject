#include "SoftWareUpgrade.h"

SoftWareUpgrade::SoftWareUpgrade(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel* label = new QLabel("软件", this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("background-color: #2b2b2b; color: #42b983; font-size: 24px; font-weight: bold;");

    layout->addWidget(label);
}

SoftWareUpgrade::~SoftWareUpgrade()
{}

