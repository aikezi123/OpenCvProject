#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "CommonLogger.h"

int main(int argc, char *argv[])
{
    // 告诉日志系统：把日志写到 D 盘！
    CommonLogger::init("D:\\OpenCvProject_Log.txt");
    LOG_INFO("程序启动，版本号: {}", "1.0.0");

    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
