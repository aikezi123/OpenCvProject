#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QSplitter>

// 引入真正的业务页面
#include "ImageShowView.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->setWindowTitle("维保软件");
    this->setFixedSize(1920, 1080);
    
    initUI();
    initPages();
    connectSignals();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initUI() {
    ui->treeWidget->setHeaderHidden(true);
    ui->treeWidget->setMinimumWidth(200);
    ui->treeWidget->setMaximumWidth(350);
}

void MainWindow::initPages() {
    QTreeWidgetItem* rootVision = new QTreeWidgetItem(ui->treeWidget, QStringList() << "图像处理");

    // 实例化刚刚用 .ui 构建的 ImageShowView
    ImageShowView* pageImage = new ImageShowView();
    int indexImage = ui->stackedWidget->addWidget(pageImage);

    QTreeWidgetItem* itemImage = new QTreeWidgetItem(rootVision, QStringList() << "单图瞳孔分析");
    itemImage->setData(0, Qt::UserRole, indexImage); // 绑定底层页码数据

    ui->treeWidget->expandAll();
    ui->treeWidget->setCurrentItem(itemImage);
    ui->stackedWidget->setCurrentIndex(indexImage);
}

void MainWindow::connectSignals() {
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged,
        this, [this](QTreeWidgetItem* current, QTreeWidgetItem* previous) {
            if (!current) return;

            QVariant data = current->data(0, Qt::UserRole);
            if (data.isValid()) {
                ui->stackedWidget->setCurrentIndex(data.toInt());
            }
        });
}