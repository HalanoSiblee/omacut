#pragma once

#include <QImage>
#include <QString>
#include <QThread>
#include <QVector>

// Generates the filmstrip thumbnails off the UI thread.
class ThumbWorker : public QThread {
    Q_OBJECT

public:
    ThumbWorker(QString path, double duration, int count, QObject *parent = nullptr)
        : QThread(parent), m_path(std::move(path)), m_duration(duration), m_count(count) {}

signals:
    void ready(const QVector<QImage> &images);

protected:
    void run() override;

private:
    QString m_path;
    double m_duration;
    int m_count;
};
