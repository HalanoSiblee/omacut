#pragma once

#include <QString>
#include <QThread>
#include <QVector>

// Fetches the video's keyframe timestamps off the UI thread (ffprobe can be
// slow on long files).
class KeyframeWorker : public QThread {
    Q_OBJECT

public:
    KeyframeWorker(QString path, QObject *parent = nullptr)
        : QThread(parent), m_path(std::move(path)) {}

signals:
    void ready(const QVector<double> &times);

protected:
    void run() override;

private:
    QString m_path;
};
