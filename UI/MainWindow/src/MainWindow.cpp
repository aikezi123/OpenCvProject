#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QSplitter>
#include "VideoTrackWidgetView.h"
#include "ImageShowView.h"
#include "FirmWareUpgradeView.h"
#include "SoftWareUpgrade.h"
#include "FrontPageView.h"

MainWindow::~MainWindow() {

}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->setWindowTitle("维保软件");
    this->setFixedSize(1920, 1080); // 锁定工业控制台标准分辨率

    initUIStyle();
    initPages();
    connectSignals();
}

void MainWindow::initUIStyle() {
    // 1. 彻底关闭系统根节点装饰
    ui->treeWidget->setRootIsDecorated(false);

    // 2. 🌟 关键：利用 Qt 原生的子节点缩进代替 QSS 的 padding-left 控制
    // 设置为 25px，这样子节点会自动向右偏移 25px，完美对齐父节点的文字
    ui->treeWidget->setIndentation(50);

    ui->treeWidget->setHeaderHidden(true);
    ui->treeWidget->setFixedWidth(230);

    // 3. 修正后的 QSS
    ui->treeWidget->setStyleSheet(QString(R"(
        /* 整个树的背景 */
        QTreeWidget {
            background-color: #ffffff;
            border: none;
            outline: none;
        }

        /* 🌟 核心：彻底抹除系统自带的左侧分支箭头和虚线占用的空间 */
        QTreeWidget::branch {
            image: none;
            border-image: none;
        }

        /* 所有项的基础样式（全局只保留一个较小的 padding） */
        QTreeWidget::item {
            font-family: 'Source Han Sans SC';
            font-size: 14px;
            font-weight: bold;
            height: 50px;
            color: #333333;
            border: none;
            padding-left: 15px; /* 全局基础左边距 */
        }

        /* 选中状态 */
        QTreeWidget::item:selected {
            background-color: #eef6ff;
            color: #00b8cc;
        }

        /* 悬停状态 */
        QTreeWidget::item:hover {
            background-color: #f5f7fa;
        }
    )"));
}

void MainWindow::initPages() {
    //——————首页————————
    addRootBusinessPage("首页", new FrontPageView(this));


    //——————图像处理模块————————
    // 建立“图像处理”分类
    QTreeWidgetItem* rootVision = addCategoryNode("图像处理");
    // 向该分类下添加具体的业务页面
    // 实例化 ImageShowView 并绑定到“单图瞳孔分析”节点
    addBusinessPage(rootVision, "单图瞳孔分析", new ImageShowView(this));
    addBusinessPage(rootVision, "相机图像采集", new VideoTrackWidgetView(this));


    //——————升级模块———————
    // 建立“升级”分类
    QTreeWidgetItem* rootUpdate = addCategoryNode("升级");
    // 增加固件升级业务页面
    addBusinessPage(rootUpdate, "固件升级", new FirmWareUpgradeView(this));
    // 增加软件升级业务界面
    addBusinessPage(rootUpdate, "软件升级", new SoftWareUpgrade(this));
    ui->treeWidget->expandAll();
}

void MainWindow::connectSignals() {
    // 【保留你原有的页面切换逻辑】
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged,
        this, [this](QTreeWidgetItem* current, QTreeWidgetItem* previous) {
            if (!current) return;
            QVariant data = current->data(0, Qt::UserRole);
            if (data.isValid()) {
                ui->stackedWidget->setCurrentIndex(data.toInt());
            }
        });

    // ==========================================
    // 🌟 新增 1：实现“单击即可展开/收缩”
    // ==========================================
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, [](QTreeWidgetItem* item, int column) {
        // 判断如果这个节点有子节点（说明它是分类父节点），就切换它的展开状态
        if (item->childCount() > 0) {
            item->setExpanded(!item->isExpanded());
        }
        });

    // ==========================================
    // 🌟 新增 2：节点展开时 -> 箭头朝上
    // ==========================================
    connect(ui->treeWidget, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem* item) {
        // 1. 获取这个节点绑定的自定义 Widget 容器
        QWidget* container = ui->treeWidget->itemWidget(item, 0);
        if (container) {
            // 2. 通过刚才设置的名字，精准找到那个箭头 Label
            QLabel* arrowLab = container->findChild<QLabel*>("arrowLabel");
            if (arrowLab) {
                // 3. 替换为“向上”的箭头图片 (请确保你的资源库里有这张图)
                arrowLab->setPixmap(QPixmap(":/Image/MainWindow/Image/MainWindow/Icon/arrow-up.png").scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }
        });

    // ==========================================
    // 🌟 新增 3：节点收缩时 -> 箭头朝下
    // ==========================================
    connect(ui->treeWidget, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem* item) {
        QWidget* container = ui->treeWidget->itemWidget(item, 0);
        if (container) {
            QLabel* arrowLab = container->findChild<QLabel*>("arrowLabel");
            if (arrowLab) {
                // 替换回“向下”的箭头图片
                arrowLab->setPixmap(QPixmap(":/Image/MainWindow/Image/MainWindow/Icon/arrow-down.png").scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }
        });
}

// 1. 插入分类根节点（带自定义右侧展开箭头）
QTreeWidgetItem* MainWindow::addCategoryNode(const QString& name) {
    // 创建挂载在 treeWidget 上的根节点
    QTreeWidgetItem* root = new QTreeWidgetItem(ui->treeWidget);

    // 创建一个自定义容器作为 Item 布局
    QWidget* container = new QWidget();

    // 🌟 核心修复 1：允许鼠标穿透！否则你点击图标和文字时，节点不会变成蓝色选中状态
    container->setAttribute(Qt::WA_TransparentForMouseEvents);
    // 确保背景透明，不遮挡 QTreeWidget 原生 QSS 定义的选中背景色
    container->setStyleSheet("background: transparent;");

    // 建立水平布局
    QHBoxLayout* layout = new QHBoxLayout(container);
    // 这里的左侧 margin 设为 0，因为整体左侧缩进已经交由 initUIStyle() 中的 setIndentation() 控制
    layout->setContentsMargins(0, 0, 20, 0);

    // 左侧图标（根据你的资源路径）
    QLabel* iconLab = new QLabel();
    iconLab->setPixmap(QPixmap(":/Image/MainWindow/Image/MainWindow/Icon/rootIcon.png").scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 中间文字
    QLabel* textLab = new QLabel(name);
    // 🌟 核心修复 2：修正了 QSS 语法，去掉了属性名中间的空格
    textLab->setStyleSheet("font-weight: bold; font-family: 'Source Han Sans SC'; font-size: 14px; color: #333333;");

    // 右侧箭头（默认向下）
    QLabel* arrowLab = new QLabel();
    // 🌟 核心修复 3：打上内部标签名，方便在 connectSignals 的展开/收缩事件里用 findChild 找到它并替换图片
    arrowLab->setObjectName("arrowLabel");
    arrowLab->setPixmap(QPixmap(":/Image/MainWindow/Image/MainWindow/Icon/arrow-down.png").scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 按顺序装配到布局中
    layout->addWidget(iconLab);
    layout->addSpacing(10); // 图标和文字之间的固定间距
    layout->addWidget(textLab);
    layout->addStretch();   // 弹簧挤压，把箭头推到最右侧
    layout->addWidget(arrowLab);

    // 将组装好的自定义 Widget 塞入刚创建的树节点中
    ui->treeWidget->setItemWidget(root, 0, container);

    return root;
}
//2.插入业务子节点并绑定页面
void MainWindow::addBusinessPage(QTreeWidgetItem* parent, const QString& name, QWidget* page) {
    if (!parent || !page) return;

    int index = ui->stackedWidget->addWidget(page);
    QTreeWidgetItem* child = new QTreeWidgetItem(parent);

    // 只需设置文字，Qt 的 setIndentation 会自动处理层级缩进
    child->setText(0, name);

    child->setData(0, Qt::UserRole, index);
}

void MainWindow::addRootBusinessPage(const QString& name, QWidget* page) {
    if (!page) return;

    int index = ui->stackedWidget->addWidget(page);
    QTreeWidgetItem* rootItem = new QTreeWidgetItem(ui->treeWidget);

    QWidget* container = new QWidget();
    // 🌟 同样需要鼠标穿透
    container->setAttribute(Qt::WA_TransparentForMouseEvents);
    container->setStyleSheet("background: transparent;");

    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 20, 0);

    // 首页的图标（你可以换成首页专属的 Icon）
    QLabel* iconLab = new QLabel();
    iconLab->setPixmap(QPixmap(":/Image/MainWindow/Image/MainWindow/Icon/rootIcon.png").scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QLabel* textLab = new QLabel(name);
    textLab->setStyleSheet("font-weight: bold; font-family: 'Source Han Sans SC'; font-size: 14px; color: #333333;");

    layout->addWidget(iconLab);
    layout->addSpacing(10);
    layout->addWidget(textLab);
    layout->addStretch(); // 首页不需要右侧箭头

    ui->treeWidget->setItemWidget(rootItem, 0, container);
    rootItem->setData(0, Qt::UserRole, index);
}