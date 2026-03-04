// Application/src/main.cpp
#include <QApplication>
#include "AppManager.h"
#include <QMetaType> // 加上头文件

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    qRegisterMetaType<cv::Mat>("cv::Mat");

    // 实例化总调度师
    AppManager appManager;

    // 让调度师去完成所有底层组装
    appManager.initialize();

    // 点火启动
    appManager.start();

    // 进入 Qt 的事件循环
    return a.exec();
}