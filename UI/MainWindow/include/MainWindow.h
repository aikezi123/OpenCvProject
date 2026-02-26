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
    void initUI();
    void initPages();
    void connectSignals();

private:
    Ui::MainWindow* ui;
};