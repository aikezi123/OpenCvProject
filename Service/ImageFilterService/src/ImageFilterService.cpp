#include "ImageFilterService.h"
#include "CommonLogger.h" // 使用最新的底座日志文件
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cstring> // 用于 std::memcpy

// =========================================================================
// 🌟 1. 核心算法：前置灰度手术 + 暗度优先扫描 (0.05秒极速版)
// =========================================================================
std::vector<Point2D> ImageFilterService::findPupilEdges(const ImageFrame& img) {
    cv::Mat mat = toCvMat(img);
    std::vector<Point2D> result;
    if (mat.empty()) return result;

    cv::Mat cleanMat = removeAlphaChannel(mat);
    cv::Mat originalGray;
    cv::cvtColor(cleanMat, originalGray, cv::COLOR_BGR2GRAY);

    // =====================================================================
    // 🚀 极速外挂 1：更激进的降维雷达 (600px 即可保证几何精度)
    // =====================================================================
    double scale = 1.0;
    int maxWorkingSize = 600;
    cv::Mat gray = originalGray.clone();

    int maxDim = std::max(gray.cols, gray.rows);
    if (maxDim > maxWorkingSize) {
        scale = static_cast<double>(maxDim) / maxWorkingSize;
        cv::resize(originalGray, gray, cv::Size(), 1.0 / scale, 1.0 / scale, cv::INTER_AREA);
    }

    // =====================================================================
    // 🚀 极速外挂 2：前置“灰度开运算”手术！(彻底消灭循环内计算)
    // MORPH_OPEN 会直接抹杀掉深色区域里的白色高光，只需执行 1 次！
    // =====================================================================
    int kernelSize = std::min(21, std::max(7, gray.rows / 15));
    if (kernelSize % 2 == 0) kernelSize++;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kernelSize, kernelSize));

    cv::Mat repairedGray;
    // 一次性吞噬所有白色高光反光
    cv::morphologyEx(gray, repairedGray, cv::MORPH_OPEN, kernel);
    cv::GaussianBlur(repairedGray, repairedGray, cv::Size(5, 5), 0);

    // =====================================================================
    // 🚀 极速外挂 3：无感循环扫描
    // =====================================================================
    double minVal;
    cv::minMaxLoc(repairedGray, &minVal);
    int startThresh = std::max(5, static_cast<int>(minVal));

    int minRadius = gray.rows / 40;
    int maxRadius = gray.rows / 2.5;

    cv::Vec3f bestCircle;
    bool found = false;

    // 此时的 repairedGray 里的瞳孔已经是完美的纯黑圆盘了
    for (int thresh = startThresh; thresh <= 200; thresh += 5) {
        cv::Mat mask;
        cv::threshold(repairedGray, mask, thresh, 255, cv::THRESH_BINARY_INV);

        // ⚠️ 注意：这里彻底删除了原来那句极其耗时的 morphEx 闭运算！
        // 现在的循环里只有极速的二值化和轮廓查找，每次循环不到 1 毫秒！

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);
            if (area < 20) continue;

            double perimeter = cv::arcLength(contour, true);
            if (perimeter == 0) continue;

            double circularity = (4.0 * CV_PI * area) / (perimeter * perimeter);
            if (circularity < 0.60) continue;

            cv::Point2f center;
            float r;
            cv::minEnclosingCircle(contour, center, r);

            if (r < minRadius || r > maxRadius) continue;

            bestCircle = { center.x, center.y, r };
            found = true;
            LOG_INFO("🎯 极速暗度防线触发！阈值 {} 截获瞳孔 -> 缩放态半径:{:.0f}, 圆度:{:.2f}",
                thresh, r, circularity);
            break;
        }
        if (found) break;
    }

    if (found) {
        // 还原坐标到原图大小
        int cx = cvRound(bestCircle[0] * scale);
        int cy = cvRound(bestCircle[1] * scale);
        int r = cvRound(bestCircle[2] * scale);

        result.push_back({ cx, cy });
        result.push_back({ r, 0 });
        LOG_INFO("⭐⭐ 极速锁定真实坐标: ({}, {}) 半径: {}", cx, cy, r);
    }
    else {
        LOG_WARNING("全军覆没：未在任何阈值层发现瞳孔特征。");
    }

    return result;
}

// =========================================================================
// 🌟 2. 兼容基类旧接口的实现
// =========================================================================

Point2D ImageFilterService::findEyeCenter(const ImageFrame& img) {
    auto edges = findPupilEdges(img);
    if (!edges.empty()) return edges[0];
    return { -1, -1 };
}

ImageFrame ImageFilterService::translateToCenter(const ImageFrame& img, Point2D center) {
    return img; // UI层已处理缩放，直接返回
}

// =========================================================================
// 🌟 3. 绘制层与数据转换工具
// =========================================================================

ImageFrame ImageFilterService::drawEdgesAndWatermark(const ImageFrame& img, const std::vector<Point2D>& edges, const std::string& watermark) {
    cv::Mat mat = toCvMat(img);
    if (mat.empty() || edges.size() < 2) return img;

    cv::Point center(edges[0].x, edges[0].y);
    int radius = edges[1].x;

    cv::circle(mat, center, radius, cv::Scalar(0, 255, 0), 2);
    int lineLen = radius / 3;
    cv::line(mat, cv::Point(center.x - lineLen, center.y), cv::Point(center.x + lineLen, center.y), cv::Scalar(0, 255, 0), 2);
    cv::line(mat, cv::Point(center.x, center.y - lineLen), cv::Point(center.x, center.y + lineLen), cv::Scalar(0, 255, 0), 2);
    cv::putText(mat, watermark, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0, 0, 255), 3);

    return toImageFrame(mat);
}

cv::Mat ImageFilterService::removeAlphaChannel(const cv::Mat& source) const {
    if (source.channels() != 4) return source.clone();
    cv::Mat result;
    std::vector<cv::Mat> channels;
    cv::split(source, channels);
    cv::cvtColor(source, result, cv::COLOR_BGRA2BGR);
    result.setTo(cv::Scalar(255, 255, 255), channels[3] == 0);
    return result;
}

cv::Mat ImageFilterService::toCvMat(const ImageFrame& img) const {
    if (!img.isValid()) return cv::Mat();
    int cvType = (img.channels == 1) ? CV_8UC1 : ((img.channels == 3) ? CV_8UC3 : CV_8UC4);
    // 智能指针获取底层裸指针
    return cv::Mat(img.height, img.width, cvType, (void*)img.data.get()).clone();
}

ImageFrame ImageFilterService::toImageFrame(const cv::Mat& mat) const {
    ImageFrame frame;
    if (mat.empty()) return frame;
    frame.width = mat.cols;
    frame.height = mat.rows;
    frame.channels = mat.channels();

    size_t dataSize = mat.total() * mat.elemSize();
    frame.data.reset(new uint8_t[dataSize]);
    std::memcpy(frame.data.get(), mat.data, dataSize);

    return frame;
}