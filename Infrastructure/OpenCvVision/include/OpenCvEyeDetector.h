#pragma once

#include "IEyeProcessor.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

// 具体的图像处理服务实现类 (专家/打工人)
class OpenCvEyeDetector : public IEyeProcessor {
public:
    OpenCvEyeDetector() = default;
    ~OpenCvEyeDetector() override = default;

    // --- 核心业务接口 ---
    std::vector<Point2D> findPupilEdges(const ImageFrame& img) override;
    ImageFrame drawEdgesAndWatermark(const ImageFrame& img, const std::vector<Point2D>& edges, const std::string& watermark) override;

    // --- 兼容基类的旧接口 ---
    Point2D findEyeCenter(const ImageFrame& img) override;
    ImageFrame translateToCenter(const ImageFrame& img, Point2D center) override;

private:
    // --- 内部数据转换助手 ---
    cv::Mat toCvMat(const ImageFrame& img) const;
    ImageFrame toImageFrame(const cv::Mat& mat) const;

    // --- 内部算法助手 ---
    cv::Mat removeAlphaChannel(const cv::Mat& source) const;
};