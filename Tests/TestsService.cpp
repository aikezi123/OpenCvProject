#define NOMINMAX 

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

// ===================================================================
// 核心算法函数：负压环检测 (严格同心圆模式，拒绝任何误判)
// ===================================================================
bool detectNegativePressureRing(const cv::Mat& src, cv::Mat& outImg, cv::Point2f& center, float& radius) {
    if (src.empty()) return false;

    // 1. 强行缩放，稳定分析尺度
    int targetWidth = 800;
    float scaleRate = (float)src.cols / targetWidth;
    cv::Mat workingImg;
    cv::resize(src, workingImg, cv::Size(targetWidth, src.rows / scaleRate));

    // 2. 灰度化与对比度极限拉伸
    cv::Mat gray;
    if (workingImg.channels() == 3) cv::cvtColor(workingImg, gray, cv::COLOR_BGR2GRAY);
    else gray = workingImg.clone();

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(3.0, cv::Size(8, 8));
    clahe->apply(gray, gray);

    // 3. 强力平滑，抹杀背景纹理
    cv::medianBlur(gray, gray, 9);

    // 4. 霍夫圆变换
    std::vector<cv::Vec3f> circles;

    // 【收紧参数】：
    double minDist = 15.0;
    double param1 = 100;             // Canny 高阈值稍微提高，屏蔽杂音
    double param2 = 30;              // 投票阈值，提高到30，拒绝牵强的假圆
    int minRadius = 15;
    int maxRadius = targetWidth / 2; // 不超过画面一半

    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, minDist, param1, param2, minRadius, maxRadius);

    outImg = src.clone();
    bool foundConcentric = false;

    // 5. 【铁血判定】：必须找到同心兄弟，否则统统不算！
    if (circles.size() >= 2) {
        for (size_t i = 0; i < circles.size(); i++) {
            for (size_t j = i + 1; j < circles.size(); j++) {
                cv::Vec3f c1 = circles[i];
                cv::Vec3f c2 = circles[j];

                // 计算圆心距离与半径差
                double dist = cv::norm(cv::Point2f(c1[0], c1[1]) - cv::Point2f(c2[0], c2[1]));
                double radDiff = std::abs(c1[2] - c2[2]);

                // 逻辑：圆心相距不到 15 像素，且半径相差超过 10 像素
                if (dist < 15.0 && radDiff > 10.0) {
                    foundConcentric = true;

                    // 找出哪个是外圈(大圆)，哪个是内圈(小圆)
                    cv::Vec3f outerCircle = (c1[2] > c2[2]) ? c1 : c2;
                    cv::Vec3f innerCircle = (c1[2] > c2[2]) ? c2 : c1;

                    // 还原到高清原图的坐标
                    center = cv::Point2f(outerCircle[0] * scaleRate, outerCircle[1] * scaleRate);
                    radius = outerCircle[2] * scaleRate;

                    cv::Point2f innerCenter(innerCircle[0] * scaleRate, innerCircle[1] * scaleRate);
                    float innerRadius = innerCircle[2] * scaleRate;

                    // 完美绘制：外圈绿，内圈黄，圆心红
                    cv::circle(outImg, center, radius, cv::Scalar(0, 255, 0), 4, cv::LINE_AA);
                    cv::circle(outImg, innerCenter, innerRadius, cv::Scalar(0, 255, 255), 2, cv::LINE_AA);
                    cv::circle(outImg, center, 6, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);

                    break; // 找到一对就收工
                }
            }
            if (foundConcentric) break;
        }
    }

    // 【极其绝情】：如果没有 foundConcentric，直接返回 false，绝不瞎画圈！
    return foundConcentric;
}

// ===================================================================
// 辅助函数：自动遍历文件夹，寻找最新修改的图片
// ===================================================================
std::string getLatestImagePath(const std::string& dirPath) {
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) return "";

    std::string latestFile = "";
    fs::file_time_type latestTime = fs::file_time_type::min();

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".jpg" || ext == ".png" || ext == ".bmp") {
                auto lastWriteTime = fs::last_write_time(entry);
                if (lastWriteTime > latestTime) {
                    latestTime = lastWriteTime;
                    latestFile = entry.path().string();
                }
            }
        }
    }
    return latestFile;
}

// ===================================================================
// 测试模块入口
// ===================================================================
int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "========================================" << std::endl;
    std::cout << " 负压环算法测试 [混合路线: Hough+同心逻辑] " << std::endl;
    std::cout << "========================================" << std::endl;

    std::string targetDir = "./CaptureData/Images";
    std::string imagePath = getLatestImagePath(targetDir);

    if (imagePath.empty()) {
        std::cerr << "[错误] 目录为空，找不到测试图片！" << std::endl;
        system("pause"); return -1;
    }

    cv::Mat src = cv::imread(imagePath);
    if (src.empty()) {
        std::cerr << "[错误] 图片加载失败！" << std::endl;
        system("pause"); return -1;
    }

    cv::Mat resultImg;
    cv::Point2f center;
    float radius;

    bool found = detectNegativePressureRing(src, resultImg, center, radius);

    cv::Mat displaySrc, displayRes;
    cv::resize(src, displaySrc, cv::Size(), 0.5, 0.5);
    cv::resize(resultImg, displayRes, cv::Size(), 0.5, 0.5);

    cv::imshow("Original Image", displaySrc);
    cv::imshow("Algorithm Result", displayRes);

    cv::waitKey(0);
    return 0;
}