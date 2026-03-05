#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

// ===================================================================
// 算法函数：负压环检测 (后续验证成功后，再搬进 Infrastructure 层)
// ===================================================================
bool detectNegativePressureRing(const cv::Mat& src, cv::Mat& outImg, cv::Point2f& center, float& radius) {
    if (src.empty()) return false;

    // 1. 转为灰度图
    cv::Mat gray;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    }
    else {
        gray = src.clone();
    }

    // 2. 极限对比度增强 (CLAHE - 拯救极暗图像的工业标配)
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(3.0, cv::Size(8, 8));
    clahe->apply(gray, gray);

    // 3. 降噪平滑 (抹平塑料高光和噪点)
    cv::medianBlur(gray, gray, 7);

    // 4. 霍夫梯度法找圆
    std::vector<cv::Vec3f> circles;

    // 【核心调参区】：针对不同高度的环，你需要重点调整 minRadius 和 maxRadius
    double minDist = gray.rows / 4;
    double param1 = 100;            // Canny 高阈值
    double param2 = 35;             // 累加器阈值 (越小找的圆越多，越大越严格，暗图建议适当调小)
    int minRadius = 50;             // 环的最小可能像素半径
    int maxRadius = 300;            // 环的最大可能像素半径

    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1, minDist, param1, param2, minRadius, maxRadius);

    outImg = src.clone(); // 拷贝原图用于画结果

    if (!circles.empty()) {
        center = cv::Point2f(circles[0][0], circles[0][1]);
        radius = circles[0][2];

        // 画出绿色的圆环和红色的圆心
        cv::circle(outImg, center, radius, cv::Scalar(0, 255, 0), 3, cv::LINE_AA);
        cv::circle(outImg, center, 3, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
        return true;
    }

    return false; // 没找到
}

// ===================================================================
// 测试模块入口：独立的 main 函数，不依赖你的主程序 UI！
// ===================================================================
int main() {
    std::cout << "--- 开启负压环视觉算法离线测试 ---" << std::endl;

    // 【修改这里】：填入你刚才捕获保存的本地图片路径 (注意斜杠方向)
    std::string imagePath = "F:/YQHCode/OpenCvProject/out/build/debug/bin/CaptureData/Images/IMG_20260305_150744_694.jpg";

    // 1. 读取本地图片
    cv::Mat src = cv::imread(imagePath);
    if (src.empty()) {
        std::cerr << "图片读取失败！请检查路径是否正确: " << imagePath << std::endl;
        return -1;
    }

    cv::Mat resultImg;
    cv::Point2f center;
    float radius;

    // 2. 执行算法
    bool found = detectNegativePressureRing(src, resultImg, center, radius);

    // 3. 打印结果
    if (found) {
        std::cout << "[成功] 找到负压环！" << std::endl;
        std::cout << " -> 圆心坐标: X=" << center.x << ", Y=" << center.y << std::endl;
        std::cout << " -> 像素半径: " << radius << std::endl;
    }
    else {
        std::cout << "[失败] 未能在图中检测到负压环，请调整算法参数。" << std::endl;
    }

    // 4. 弹出 OpenCV 原生窗口展示结果 (方便肉眼查验)
    // 缩小一下显示比例，防止图片太大超出屏幕
    cv::Mat displaySrc, displayRes;
    cv::resize(src, displaySrc, cv::Size(), 0.5, 0.5);
    cv::resize(resultImg, displayRes, cv::Size(), 0.5, 0.5);

    cv::imshow("Original Image", displaySrc);
    cv::imshow("Algorithm Result", displayRes);

    std::cout << "按键盘任意键退出测试..." << std::endl;
    cv::waitKey(0); // 必须加这个，否则窗口一闪而过

    return 0;
}