#include "OpenCvDemoView.h"

OpenCvDemoView::OpenCvDemoView(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	CvMatCreat();
}

OpenCvDemoView::~OpenCvDemoView()
{}

void OpenCvDemoView::CvMatCreat() {
	//cv::Mat a;
	//a = cv::imread("F:/YQHCode/OpenCvProject/UI/Resource/Image/ImageProcessing/EyeProcessing/cars.jpg");
	//int rows = a.rows;
	//int cols = a.cols;
	//cv::Mat b(a, cv::Range(0, 0.5 * rows), cv::Range(0, 0.5 * cols));
	//displayMatOnLabel(a, ui.label_display1);
	//displayMatOnLabel(b, ui.label_display2);

	uchar a[8] = { 5,6,7,8,1,2,3,4 };
	cv::Mat b = cv::Mat(2, 2, CV_8UC1, a);
	cv::Mat c = cv::Mat(2, 4, CV_8UC1, a);
	displayMatOnLabel(b, ui.label_display1);
	displayMatOnLabel(c, ui.label_display2);
}

void OpenCvDemoView::displayMatOnLabel(cv::Mat mat, QLabel* label) {
	if (mat.empty()) {
		return;
	}
	cv::Mat rgbMat;

	//1.颜色空间转换：BGR->RGB
	if (mat.channels() == 3) {
		cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);  //三通道，OpenCv的mat默认是BGR，但是Qt中使用的是RGB，因此需要进行通道转换
	}
	else {
		rgbMat = mat;  //如果是单通道灰度图则不需要转换
	}

	//2.将cv::Mat构造为QImage
	//rgbMat.data返回unchar*类型指针，指向图像像素数据在内存中的第一个字节，类似数组中对数组取地址
	//rgbMat.step:数据的步长，表示图像中每一行像素所占用的字节数。
		//很多人误以为step = 单个通道所占字节数 × 列数 × 通道数，但实际更加复杂
		//实际上：	  step = 单个通道所占字节数 × 列数 × 通道数 + 填充字节（Padding）
		//由于CPU读取内存，要求每一行数据起始地址是4或8的倍数。如果不是，就会自动在每一行末尾填充上几个空字节
		//如果不填充，将Mat转换为QImage时，图像就会变“斜”或者花屏
		//例如： 3×3像素RGB彩色图像(CV_8UC3)，每一行有1字节×3通道×3列 = 9字节。假设系统4字节对齐，则9之后4的倍数为12，需要填充3个字节。
	QImage img(
		rgbMat.data,
		rgbMat.cols,
		rgbMat.rows,
		static_cast<int>(rgbMat.step),
		(mat.channels() == 3) ? QImage::Format_RGB888 : QImage::Format_Grayscale8
	);

	// 3.设置 QPixmap，并开启 QLabel 的自动缩放属性
	label->setPixmap(QPixmap::fromImage(img));
	label->setScaledContents(true); // 让 QLabel 自动把图片撑满自己的大小
}