#pragma once
#include <QObject>
#include <QImage>
#include <QString>
#include <memory>
// 引入业务层主板
#include "TargetRecognition.h" 

class EyeWorker : public QObject {
    Q_OBJECT
public:
    explicit EyeWorker(QObject* parent = nullptr);
    ~EyeWorker();

public slots:
    // 接收来自 UI 的单张图片路径并处理
    void processImageFromFile(const QString& filePath);

signals:
    // 处理完一帧图后，抛给主界面 (UI)
    void frameProcessed(QImage image);
    // 遇到错误时通知主界面 (UI)
    void errorOccurred(QString errorMsg);

private:
    std::shared_ptr<TargetRecognition> m_business;
};