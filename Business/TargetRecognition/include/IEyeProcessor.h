#pragma once
#include "EyeDataTypes.h"

// 业务层定义的底层标准插槽
class IEyeProcessor {
public:
    virtual ~IEyeProcessor() = default;

    // 动作1：寻找眼球中心
    virtual Point2D findEyeCenter(const ImageFrame& img) = 0;

    // 动作2：图像平移 (将指定的中心点移动到图像正中心)
    virtual ImageFrame translateToCenter(const ImageFrame& img, Point2D currentCenter) = 0;

    // 动作3：寻找瞳孔边缘轮廓
    virtual std::vector<Point2D> findPupilEdges(const ImageFrame& img) = 0;

    // 动作4：绘制边缘并添加水印
    virtual ImageFrame drawEdgesAndWatermark(const ImageFrame& img,
        const std::vector<Point2D>& edges,
        const std::string& watermarkText) = 0;
};