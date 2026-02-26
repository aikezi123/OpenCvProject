#pragma once
#include <QWidget>
#include <QThread>
#include <QImage>
#include "EyeWorker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ImageShowView; }
QT_END_NAMESPACE

// 独立的单图识别页面
class ImageShowView : public QWidget {
    Q_OBJECT

public:
    explicit ImageShowView(QWidget* parent = nullptr);
    ~ImageShowView();

signals:
    void requestProcessImage(const QString& filePath);

private:
    void signalSlotInit();
    void updateImageDisplay();
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::ImageShowView* ui;

    QImage m_currentImage;
    QThread* m_workerThread;
    EyeWorker* m_worker;
};