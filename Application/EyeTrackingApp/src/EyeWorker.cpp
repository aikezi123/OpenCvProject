#include "EyeWorker.h"
#include <QFile>
#include <QByteArray>
#include <opencv2/opencv.hpp>
#include <cstring>
#include "CommonLogger.h"

// 引入底层干活的工具
#include "OpenCvEyeDetector.h"

// 【核心】：在构造函数中完成依赖注入装配
EyeWorker::EyeWorker(QObject* parent) : QObject(parent) {
    // 1. 造一把 OpenCV 的干活锤子 (Service层)
    auto cvService = std::make_shared<OpenCvEyeDetector>();
    // 2. 把锤子塞给业务包工头 (Business层)
    m_business = std::make_shared<TargetRecognition>(cvService);
}

EyeWorker::~EyeWorker() {}

void EyeWorker::processImageFromFile(const QString& filePath) {
    // 1. 使用 QFile 读取二进制，然后用 OpenCV 解码 (完美解决中文路径)
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("无法打开文件：" + filePath);
        return;
    }
    QByteArray fileData = file.readAll();
    file.close();

    std::vector<uchar> buf(fileData.begin(), fileData.end());
    cv::Mat cvFrame = cv::imdecode(buf, cv::IMREAD_COLOR);

    if (cvFrame.empty()) {
        emit errorOccurred("图像解码失败，请检查图片格式！");
        return;
    }

    // 2. 【DTO 数据转换】将 cv::Mat 转为纯净的 ImageFrame
    ImageFrame rawFrame;
    rawFrame.width = cvFrame.cols;
    rawFrame.height = cvFrame.rows;
    rawFrame.channels = cvFrame.channels();
    size_t dataSize = cvFrame.total() * cvFrame.elemSize();
    rawFrame.data = std::shared_ptr<uint8_t[]>(new uint8_t[dataSize]);
    std::memcpy(rawFrame.data.get(), cvFrame.data, dataSize);

    // 3. 【核心调用】跑业务流水线！
    ImageFrame resultFrame = m_business->processSingleFrame(rawFrame);

    // 4. 【DTO 数据转换】将处理完的 ImageFrame 转为 QImage
    if (resultFrame.isValid()) {
        QImage::Format fmt = (resultFrame.channels == 3) ? QImage::Format_BGR888 : QImage::Format_Grayscale8;
        QImage qImg(resultFrame.data.get(), resultFrame.width, resultFrame.height, resultFrame.width * resultFrame.channels, fmt);

        // 深拷贝发给 UI 线程
        emit frameProcessed(qImg.copy());
    }
    LOG_INFO("图像解码成功，正在交给业务层处理...");
}