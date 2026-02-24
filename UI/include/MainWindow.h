#pragma once
#include <QMainWindow>
#include <QThread>
#include "EyeWorker.h"
#include <QImage>
#include <QResizeEvent> // 别忘了包含这个头文件

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; } // <--- 【修改点 1】：这里的声明要加上 Class
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void signalSlotInit();   //信号槽初始化函数，专门用来把 UI 的信号和 Worker 的槽连接起来
    

signals:
    void requestProcessImage(const QString& filePath);


private:
    void displayImage();       //显示图像的函数，负责把 QImage 转成 QPixmap 显示在 QLabel 上
    void updateImageDisplay(); // 抽离出一个专门用来刷新界面的函数
    void resizeEvent(QResizeEvent* event) override;// Qt 的魔法函数：只要窗口大小发生变化，就会自动触发它

private:
    Ui::MainWindowClass* ui; // <--- 【修改点 2】：报错的真凶！这里的指针类型必须加上 Class

    QImage m_currentImage; // 1. 用来保存业务层传过来的原始高清大图
    QThread* m_workerThread;
    EyeWorker* m_worker;
};