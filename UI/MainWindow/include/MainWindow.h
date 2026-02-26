#pragma once
#include <QMainWindow>
#include <QTreeWidget>
#include <QStackedWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    // 将初始化逻辑拆分为三个维度，符合 Layer 5 UI 层的高内聚要求
    void initUI();          // 负责静态界面属性（尺寸、树控件宽度等）
    void initPages();       // 负责业务页面装载与树节点绑定
    void connectSignals();  // 负责信号槽连接

    //——————新增的组件装配接口——————
    //1.插入分类根节点
    QTreeWidgetItem* addCategoryNode(const QString& name);

    //2.插入业务子节点并绑定页面
    void addBusinessPage(QTreeWidgetItem* parent, const QString& name, QWidget* page);

private:
    Ui::MainWindow* ui;
};