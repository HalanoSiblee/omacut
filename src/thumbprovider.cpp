#include "thumbprovider.h"

#include <QMutexLocker>

void ThumbProvider::setImages(const QVector<QImage> &images) {
    QMutexLocker lock(&m_mutex);
    m_images = images;
}

void ThumbProvider::setImage(int index, const QImage &image) {
    if (index < 0)
        return;

    QMutexLocker lock(&m_mutex);
    if (index >= m_images.size())
        m_images.resize(index + 1);
    m_images[index] = image;
}

QImage ThumbProvider::requestImage(const QString &id, QSize *size, const QSize &requested) {
    // id looks like "<video-revision>/<index>"; only the index maps to storage.
    const int index = id.section('/', -1).toInt();

    QMutexLocker lock(&m_mutex);
    if (index < 0 || index >= m_images.size())
        return {};

    QImage img = m_images.at(index);
    if (img.isNull())
        return {};
    if (size)
        *size = img.size();
    if (requested.width() > 0 && requested.height() > 0)
        img = img.scaled(requested, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return img;
}
