#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "CameraView.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();


// ——————接口函数———————
public:
    CameraView* getCameraView() const { return m_cameraView; }

private:
    // 初始化流
    void initUIStyle();
    void initPages();
    void connectSignals();

    // ==========================================
    // 核心业务层：负责逻辑建树和页面路由绑定
    // ==========================================
    // 1. 插入分类根节点 (纯逻辑，无跳转)
    QTreeWidgetItem* addCategoryNode(const QString& name, const QString& iconPath = "");

    // 2. 插入业务子节点并绑定页面 (纯逻辑，带跳转)
    void addBusinessPage(QTreeWidgetItem* parent, const QString& name, QWidget* page);

    // 3. 插入业务根节点 (既是根节点又跳转页面，如“首页”)
    void addRootBusinessPage(const QString& name, QWidget* page, const QString& iconPath = "");

    // ==========================================
    // 纯 UI 美化层：只负责给节点绘制图标和箭头
    // ==========================================
    void decorateParentNodeUI(QTreeWidgetItem* item, const QString& name, const QString& iconPath, bool hasArrow);

private:
    Ui::MainWindow* ui;
    CameraView* m_cameraView = nullptr;
};

#endif // MAINWINDOW_H