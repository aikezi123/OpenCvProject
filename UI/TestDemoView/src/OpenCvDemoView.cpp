#include "OpenCvDemoView.h"
#include <QDebug>

OpenCvDemoView::OpenCvDemoView(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// 1. 实例化定时器
	m_timer = new QTimer(this);

	// 2. 将定时器的 timeout 信号连接到读取图像的槽函数
	connect(m_timer, &QTimer::timeout, this, &OpenCvDemoView::readFrame);

	// 3. 启动相机
	startCamera();
}

OpenCvDemoView::~OpenCvDemoView()
{
	// 退出程序时安全释放资源
	if (m_timer->isActive()) {
		m_timer->stop();
	}
	if (m_video.isOpened()) {
		m_video.release();
	}
}

void OpenCvDemoView::startCamera() {
	m_video.open(0);

	if (!m_video.isOpened()) {
		qDebug() << "打开摄像头失败";
		return;
	}

	// 摄像头打开成功后，初始化控制面板
	initCameraControls();

	m_timer->start(33);
}

// ==========================================
// 【新增】专门处理相机属性初始化的模块
// ==========================================
void OpenCvDemoView::initCameraControls() {
	// --- 1. 亮度 (Brightness) ---
	ui.brightnessSlider->setRange(0, 100);
	int brightness = static_cast<int>(m_video.get(cv::CAP_PROP_BRIGHTNESS));
	ui.brightnessSlider->setValue(brightness);
	ui.label_brightnessValue->setText(QString::number(brightness));
	connect(ui.brightnessSlider, &QSlider::valueChanged, this, &OpenCvDemoView::onBrightnessChanged);

	// --- 2. 曝光 (Exposure) ---
	// Windows UVC 相机的曝光值通常是负数（比如 -4 代表 2的-4次方秒）
	// 注意：有些相机需要先关闭“自动曝光”才能手动调节
	m_video.set(cv::CAP_PROP_AUTO_EXPOSURE, 0); // 尝试关闭自动曝光 (0 或 0.25)

	ui.exposureSlider->setRange(-10, 0); // 曝光范围通常在负数区间
	int exposure = static_cast<int>(m_video.get(cv::CAP_PROP_EXPOSURE));
	ui.exposureSlider->setValue(exposure);
	ui.label_exposureValue->setText(QString::number(exposure));
	connect(ui.exposureSlider, &QSlider::valueChanged, this, &OpenCvDemoView::onExposureChanged);

	// --- 3. 对比度 (Contrast) ---
	ui.contrastSlider->setRange(0, 100);
	int contrast = static_cast<int>(m_video.get(cv::CAP_PROP_CONTRAST));
	ui.contrastSlider->setValue(contrast);
	ui.label_contrastValue->setText(QString::number(contrast));
	connect(ui.contrastSlider, &QSlider::valueChanged, this, &OpenCvDemoView::onContrastChanged);
}

// 【新增】滑块拖动时的响应函数
void OpenCvDemoView::onBrightnessChanged(int value) {
	if (m_video.isOpened()) {
		m_video.set(cv::CAP_PROP_BRIGHTNESS, value);

		// 【新增】实时更新 Label 显示的数字
		ui.label_brightnessValue->setText(QString::number(value));

		qDebug() << "当前设置亮度为:" << value;
	}
}

void OpenCvDemoView::readFrame() {
	cv::Mat img;

	// 判断能否继续从摄像头读出一帧图像
	if (!m_video.read(img) || img.empty()) {
		qDebug() << "摄像头断开连接或未获取到图像";
		m_timer->stop(); // 出现异常时停止定时器
		return;
	}

	// 调用您的函数进行显示。注意：这里是 ui.label_display1 (使用点运算符，因为 ui 是对象)
	displayMatOnLabel(img, ui.label_display1);
}

// 下方完全保留您写的完美函数
void OpenCvDemoView::displayMatOnLabel(cv::Mat mat, QLabel* label) {
	if (mat.empty()) {
		return;
	}
	cv::Mat rgbMat;

	//1.颜色空间转换：BGR->RGB
	if (mat.channels() == 3) {
		cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
	}
	else {
		rgbMat = mat;
	}

	//2.将cv::Mat构造为QImage
	QImage img(
		rgbMat.data,
		rgbMat.cols,
		rgbMat.rows,
		static_cast<int>(rgbMat.step),
		(mat.channels() == 3) ? QImage::Format_RGB888 : QImage::Format_Grayscale8
	);

	// 3.设置 QPixmap，并开启 QLabel 的自动缩放属性
	label->setPixmap(QPixmap::fromImage(img));
	label->setScaledContents(true);
}

void OpenCvDemoView::onExposureChanged(int value) {
	if (m_video.isOpened()) {
		m_video.set(cv::CAP_PROP_EXPOSURE, value);
		ui.label_exposureValue->setText(QString::number(value));
	}
}

void OpenCvDemoView::onContrastChanged(int value) {
	if (m_video.isOpened()) {
		m_video.set(cv::CAP_PROP_CONTRAST, value);
		ui.label_contrastValue->setText(QString::number(value));
	}
}