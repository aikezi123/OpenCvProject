#include <QApplication>
#include "MainWindow.h"

// 极其重要：必须是 (int argc, char *argv[])，绝不能是 int main() 无参！
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // 【新增核心代码】：手动初始化 UI 层的资源文件！
    // 括号里的名字必须与你的 .qrc 文件名完全一致（你的文件叫 Resource.qrc）
    Q_INIT_RESOURCE(Resource);

    MainWindow w;
    w.show();

    return a.exec();
}