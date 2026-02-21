#pragma once
// 【反向依赖】：必须包含业务层的头文件，去实现它定下的规矩
#include "IEyeProcessor.h" 
#include <opencv2/opencv.hpp>

class ImageFilterService : public IEyeProcessor {
private:
    // 核心内存转换工具：ImageFrame -> cv::Mat
    cv::Mat toCvMat(const ImageFrame& frame);

    // 核心内存转换工具：cv::Mat -> ImageFrame
    ImageFrame toImageFrame(const cv::Mat& mat);

public:
    ImageFilterService() = default;
    ~ImageFilterService() override = default;

    // 实现基类的纯虚函数
    Point2D findEyeCenter(const ImageFrame& img) override;
    ImageFrame translateToCenter(const ImageFrame& img, Point2D currentCenter) override;
    std::vector<Point2D> findPupilEdges(const ImageFrame& img) override;
    ImageFrame drawEdgesAndWatermark(const ImageFrame& img,
        const std::vector<Point2D>& edges,
        const std::string& watermarkText) override;
};