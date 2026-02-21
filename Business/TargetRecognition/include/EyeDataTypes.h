#pragma once
#include <memory>
#include <vector>
#include <string>
#include <cstdint> // 为了使用 uint8_t

// 中立的坐标点
struct Point2D {
    int x;
    int y;
};

// 中立的图像结构（智能指针安全接管内存）
struct ImageFrame {
    std::shared_ptr<uint8_t[]> data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;

    // 辅助判定图像是否有效
    bool isValid() const {
        return data != nullptr && width > 0 && height > 0;
    }
};