#include <QApplication>
#include "MainWindow.h"

// 极其重要：必须是 (int argc, char *argv[])，绝不能是 int main() 无参！
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}