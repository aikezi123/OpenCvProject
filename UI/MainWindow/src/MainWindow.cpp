#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QSplitter>
#include "VideoTrackWidgetView.h"

// 引入垂直业务切片：图像处理模块
#include "ImageShowView.h"

MainWindow::~MainWindow() {

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->setWindowTitle("维保软件");
    this->setFixedSize(1920, 1080); // 锁定工业控制台标准分辨率

    initUI();
    initPages();
    connectSignals();
}

void MainWindow::initUI() {
    ui->treeWidget->setHeaderHidden(true); // 隐藏表头，使导航栏更像菜单
    ui->treeWidget->setMinimumWidth(200);
    ui->treeWidget->setMaximumWidth(350);
}

void MainWindow::initPages() {
    //——————图像处理模块————————
    // 建立“图像处理”分类
    QTreeWidgetItem* rootVision = addCategoryNode("图像处理");
    // 向该分类下添加具体的业务页面
    // 实例化 ImageShowView 并绑定到“单图瞳孔分析”节点
    addBusinessPage(rootVision, "单图瞳孔分析", new ImageShowView());
    addBusinessPage(rootVision, "相机图像采集", new VideoTrackWidgetView());


    //——————升级模块———————
    // 固件升级
    QTreeWidgetItem* rootUpdate = addCategoryNode("升级");
    //// 软件升级
    //addBusinessPage(rootUpdate, "固件升级", );

    ui->treeWidget->expandAll();
}

void MainWindow::connectSignals() {
    // 🌟 导航切换逻辑：
    // 使用 currentItemChanged 而非 itemClicked 的好处是：
    // 无论是鼠标点击还是键盘上下键切换，都能触发页面跳转
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged,
        this, [this](QTreeWidgetItem* current, QTreeWidgetItem* previous) {
            if (!current) return;

            // 提取初始化时存入的页码数据
            QVariant data = current->data(0, Qt::UserRole);
            if (data.isValid()) {
                // 执行页面跳转，实现“树导航 - 容器切换”的解耦
                ui->stackedWidget->setCurrentIndex(data.toInt());
            }
        });
}

//1.插入分类根节点
QTreeWidgetItem* MainWindow::addCategoryNode(const QString& name) {
    QTreeWidgetItem* root = new QTreeWidgetItem(ui->treeWidget, QStringList() << name);
    // 根节点通常不绑定页面索引，确保点击它时不会发生跳转
    return root;
}

//2.插入业务子节点并绑定页面
void MainWindow::addBusinessPage(QTreeWidgetItem* parent, const QString& name, QWidget* page) {
    if (!parent || !page) return;

    // 1. 将页面对象塞进堆栈容器，并获取其唯一的物理索引
    int index = ui->stackedWidget->addWidget(page);

    // 2. 在指定父节点下创建子项
    QTreeWidgetItem* child = new QTreeWidgetItem(parent, QStringList() << name);

    // 3. 执行“一一对应”的元数据绑定
    child->setData(0, Qt::UserRole, index);
}