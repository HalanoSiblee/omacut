#include "thumbworker.h"

#include "ffmpeg.h"

void ThumbWorker::run() {
    for (int i = 0; i < m_count; ++i) {
        if (isInterruptionRequested())
            return;
        const double t = m_duration * (i + 0.5) / m_count;
        const QImage img = ffmpeg::thumbnail(m_path, t);
        if (isInterruptionRequested())
            return;
        emit thumbReady(i, img);
    }
}
