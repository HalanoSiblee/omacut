#pragma once

#include <QImage>
#include <QMutex>
#include <QQuickImageProvider>
#include <QVector>

// Serves the filmstrip thumbnails to QML. QML requests them by index, e.g.
//   Image { source: "image://thumbs/3?<revision>" }
// The optional ?<revision> query busts QML's image cache when a new video
// is loaded and the same indices point at different frames.
class ThumbProvider : public QQuickImageProvider {
public:
    ThumbProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    void setImages(const QVector<QImage> &images);

    QImage requestImage(const QString &id, QSize *size, const QSize &requested) override;

private:
    QVector<QImage> m_images;
    QMutex m_mutex;
};
