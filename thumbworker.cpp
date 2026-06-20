#include "thumbworker.h"

#include "ffmpeg.h"

void ThumbWorker::run() {
    QVector<QImage> images;
    for (int i = 0; i < m_count; ++i) {
        const double t = m_duration * (i + 0.5) / m_count;
        const QImage img = ffmpeg::thumbnail(m_path, t);
        if (!img.isNull())
            images.append(img);
    }
    emit ready(images);
}
