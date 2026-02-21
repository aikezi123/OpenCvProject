#include "TargetRecognition.h"

TargetRecognition::TargetRecognition(std::shared_ptr<IEyeProcessor> processor)
    : m_processor(processor) {
}

ImageFrame TargetRecognition::processSingleFrame(const ImageFrame& rawCameraImg) {
    // 异常拦截：如果没有注入工具或者图像为空，直接原样返回
    if (!m_processor || !rawCameraImg.isValid()) {
        return rawCameraImg;
    }

    // SOP 流水线处理（纯粹的业务逻辑，就像大白话一样清晰）

    // 第一步：识别出眼球的中心
    Point2D center = m_processor->findEyeCenter(rawCameraImg);

    // 第二步：将图像进行移动，将眼球中心放在图像中心
    ImageFrame centeredImg = m_processor->translateToCenter(rawCameraImg, center);

    // 第三步：识别出瞳孔黑色的边缘
    std::vector<Point2D> edges = m_processor->findPupilEdges(centeredImg);

    // 第四步：添加水印并画出边缘
    ImageFrame finalImg = m_processor->drawEdgesAndWatermark(centeredImg, edges, "EYE_TRACKING_OK");

    return finalImg;
}