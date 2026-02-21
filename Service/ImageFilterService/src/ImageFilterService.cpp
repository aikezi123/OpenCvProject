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

std::vector<Point2D> ImageFilterService::findPupilEdges(const ImageFrame& img) {
    cv::Mat mat = toCvMat(img);
    std::vector<Point2D> resultEdges;
    if (mat.empty()) return resultEdges;

    // 这里通常会进行：灰度化 -> 二值化 -> 查找轮廓
    cv::Mat gray, thresh;
    if (mat.channels() == 3) {
        cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
    }
    else {
        gray = mat;
    }

    // 寻找黑色的瞳孔区域 (阈值设为 50 左右)
    cv::threshold(gray, thresh, 50, 255, cv::THRESH_BINARY_INV);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 假设最大的轮廓就是瞳孔
    if (!contours.empty()) {
        for (const auto& pt : contours[0]) {
            resultEdges.push_back(Point2D{ pt.x, pt.y });
        }
    }
    return resultEdges;
}

ImageFrame ImageFilterService::drawEdgesAndWatermark(const ImageFrame& img,
    const std::vector<Point2D>& edges,
    const std::string& watermarkText) {
    cv::Mat mat = toCvMat(img);
    if (mat.empty()) return img;

    // 1. 绘制边缘 (画绿线)
    for (const auto& pt : edges) {
        cv::circle(mat, cv::Point(pt.x, pt.y), 1, cv::Scalar(0, 255, 0), -1);
    }

    // 2. 添加水印 (画红字)
    cv::putText(mat, watermarkText, cv::Point(20, 40),
        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);

    return toImageFrame(mat);
}