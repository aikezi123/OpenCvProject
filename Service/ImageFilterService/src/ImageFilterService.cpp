#include "ImageFilterService.h"
#include <cstring> // std::memcpy 需要

// ==========================================================
// 内存隔离工具实现
// ==========================================================
cv::Mat ImageFilterService::toCvMat(const ImageFrame& frame) {
    if (!frame.isValid()) return cv::Mat();
    int type = (frame.channels == 3) ? CV_8UC3 : CV_8UC1;
    // 使用 clone() 进行深拷贝，避免 OpenCV 内部的滤波操作破坏原业务数据
    return cv::Mat(frame.height, frame.width, type, frame.data.get()).clone();
}

ImageFrame ImageFilterService::toImageFrame(const cv::Mat& mat) {
    ImageFrame frame;
    if (mat.empty()) return frame;

    frame.width = mat.cols;
    frame.height = mat.rows;
    frame.channels = mat.channels();

    size_t dataSize = mat.total() * mat.elemSize();
    // C++17 语法：分配数组内存交由 shared_ptr 管理
    frame.data = std::shared_ptr<uint8_t[]>(new uint8_t[dataSize]);
    std::memcpy(frame.data.get(), mat.data, dataSize);

    return frame;
}

// ==========================================================
// 业务接口的 OpenCV 落地实现
// ==========================================================
Point2D ImageFilterService::findEyeCenter(const ImageFrame& img) {
    cv::Mat mat = toCvMat(img);
    if (mat.empty()) return Point2D{ 0, 0 };

    // 假设用简单的图像矩 (Moments) 或者霍夫圆查找中心
    // 这里为了演示，假设中心点就在画面几何正中心偏移一点的位置
    return Point2D{ mat.cols / 2 + 10, mat.rows / 2 - 5 };
}

ImageFrame ImageFilterService::translateToCenter(const ImageFrame& img, Point2D currentCenter) {
    cv::Mat mat = toCvMat(img);
    if (mat.empty()) return img;

    // 计算需要平移的距离 dx, dy
    int targetX = mat.cols / 2;
    int targetY = mat.rows / 2;
    int dx = targetX - currentCenter.x;
    int dy = targetY - currentCenter.y;

    // 构造 OpenCV 的仿射变换平移矩阵 M = [1 0 dx; 0 1 dy]
    cv::Mat M = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);

    cv::Mat translatedMat;
    cv::warpAffine(mat, translatedMat, M, mat.size());

    return toImageFrame(translatedMat);
}

// 1. 算法升级：使用“霍夫圆变换”自动捕捉瞳孔位置
std::vector<Point2D> ImageFilterService::findPupilEdges(const ImageFrame& img) {
    cv::Mat mat = toCvMat(img); // 把干净的 DTO 转成 OpenCV 认识的 Mat
    std::vector<Point2D> result;
    if (mat.empty()) return result;

    cv::Mat gray;
    // 强制转为灰度图（识别瞳孔必备）
    if (mat.channels() == 3) {
        cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
    }
    else {
        gray = mat.clone();
    }

    // 高斯模糊：去掉红血丝等噪点的干扰
    cv::GaussianBlur(gray, gray, cv::Size(9, 9), 2, 2);

    // 核心算法：霍夫梯度法找圆
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1,
        gray.rows / 8, // 圆心之间的最小距离
        100,           // Canny 边缘检测的高阈值
        30,            // 累加器阈值（越小检测到的假圆越多）
        20,            // 瞳孔最小半径
        gray.rows / 3  // 瞳孔最大半径
    );

    if (!circles.empty()) {
        // 取第一个圆（得分最高、最明显的瞳孔）
        int cx = cvRound(circles[0][0]);
        int cy = cvRound(circles[0][1]);
        int r = cvRound(circles[0][2]);

        // 巧妙的传递：把圆心放在第0个点，半径放在第1个点的x里
        result.push_back({ cx, cy });
        result.push_back({ r, 0 });
    }

    return result;
}

// 2. 绘图升级：在刚才找到的位置画上霸气的十字准星
ImageFrame ImageFilterService::drawEdgesAndWatermark(const ImageFrame& img, const std::vector<Point2D>& edges, const std::string& watermark) {
    cv::Mat mat = toCvMat(img);
    if (mat.empty()) return img;

    // 必须转成彩色通道，否则绿色的十字画不出来
    if (mat.channels() == 1) {
        cv::cvtColor(mat, mat, cv::COLOR_GRAY2BGR);
    }

    // 如果成功找到了瞳孔（有圆心和半径 2 个数据）
    if (edges.size() >= 2) {
        int cx = edges[0].x;
        int cy = edges[0].y;
        int r = edges[1].x;

        // 步骤 A：画一个绿色的圈，完美框住瞳孔 (Scalar(B, G, R) -> 0, 255, 0 是纯绿)
        cv::circle(mat, cv::Point(cx, cy), r, cv::Scalar(0, 255, 0), 2);

        // 步骤 B：在圆心画绿色的十字准星
        int crossSize = 20; // 十字准星的一半长度
        // 画横线
        cv::line(mat, cv::Point(cx - crossSize, cy), cv::Point(cx + crossSize, cy), cv::Scalar(0, 255, 0), 2);
        // 画竖线
        cv::line(mat, cv::Point(cx, cy - crossSize), cv::Point(cx, cy + crossSize), cv::Scalar(0, 255, 0), 2);
    }

    // 步骤 C：打上我们之前的红字水印
    cv::putText(mat, watermark, cv::Point(30, 50), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 0, 255), 3);

    return toImageFrame(mat); // 完工，转回纯净的 DTO 扔给业务层
}