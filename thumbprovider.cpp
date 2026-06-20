#include "thumbprovider.h"

#include <QMutexLocker>

void ThumbProvider::setImages(const QVector<QImage> &images) {
    QMutexLocker lock(&m_mutex);
    m_images = images;
}

QImage ThumbProvider::requestImage(const QString &id, QSize *size, const QSize &requested) {
    // id looks like "3" or "3?7" — take the leading index, drop any revision.
    const int index = id.section('?', 0, 0).toInt();

    QMutexLocker lock(&m_mutex);
    if (index < 0 || index >= m_images.size())
        return {};

    QImage img = m_images.at(index);
    if (size)
        *size = img.size();
    if (requested.width() > 0 && requested.height() > 0)
        img = img.scaled(requested, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    return img;
}
