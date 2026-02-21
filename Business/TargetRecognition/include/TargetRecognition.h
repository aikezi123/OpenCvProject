#pragma once
#include "IEyeProcessor.h"
#include <memory>

class TargetRecognition {
private:
    std::shared_ptr<IEyeProcessor> m_processor; // 依赖注入的接口（防弹衣）

public:
    // 强制构造函数注入：没有底层工具，业务无法运行
    explicit TargetRecognition(std::shared_ptr<IEyeProcessor> processor);

    // 核心业务：处理单帧画面
    ImageFrame processSingleFrame(const ImageFrame& rawCameraImg);
};