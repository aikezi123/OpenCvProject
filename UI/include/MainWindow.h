#pragma once
#include <QMainWindow>
#include <QThread>
#include "EyeWorker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; } // <--- 【修改点 1】：这里的声明要加上 Class
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void requestProcessImage(const QString& filePath);

private:
    Ui::MainWindowClass* ui; // <--- 【修改点 2】：报错的真凶！这里的指针类型必须加上 Class

    QThread* m_workerThread;
    EyeWorker* m_worker;
};