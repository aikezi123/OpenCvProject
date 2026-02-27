#include "FrontPageView.h"

FrontPageView::FrontPageView(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	initUiStyle();

}

FrontPageView::~FrontPageView()
{

}

void FrontPageView::initUiStyle() {

	QString style = QString(R"(
		QLabel#label_display {
			border-image: url(:/Image/MainWindow/Image/MainWindow/UI/FrontPage.png) 0 0 0 0 stretch stretch;
		}
		QPushButton#pushButton_SecurityLogicSate {
			border-image: url(:/Image/MainWindow/Image/MainWindow/Button/SecurityLogicSate.png) 0 0 0 0 stretch stretch;
		}	
		QPushButton#pushButton_Mode	{
			border-image: url(:/Image/MainWindow/Image/MainWindow/Button/Mode.png) 0 0 0 0 stretch stretch;
		}
		QPushButton#pushButton_LaserState {
			border-image: url(:/Image/MainWindow/Image/MainWindow/Button/LaserState.png) 0 0 0 0 stretch stretch;
		}
		QPushButton#pushButton_TreatmentLightState {
			border-image: url(:/Image/MainWindow/Image/MainWindow/Button/TreatmentLightState.png) 0 0 0 0 stretch stretch;
		}
		QPushButton#pushButton_unknownEnergy {
			border-image: url(:/Image/MainWindow/Image/MainWindow/Button/unknownEnergy.png) 0 0 0 0 stretch stretch;
		}
		QPushButton#pushButton_RemoteDesktop {
			border-image: url(:/Image/MainWindow/Image/MainWindow/Button/RemoteDesktop.png) 0 0 0 0 stretch stretch;
		}
	)");
	this->setStyleSheet(style);

	ui.label_display->setFixedSize(1644, 890);
	ui.pushButton_SecurityLogicSate->setFixedSize(220, 60);
	ui.pushButton_Mode->setFixedSize(220, 60);
	ui.pushButton_LaserState->setFixedSize(220, 60);
	ui.pushButton_TreatmentLightState->setFixedSize(220, 60);
	ui.pushButton_unknownEnergy->setFixedSize(220, 60);
	ui.pushButton_RemoteDesktop->setFixedSize(220, 60);

	
}

