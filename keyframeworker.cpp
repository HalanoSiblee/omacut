#include "keyframeworker.h"

#include "ffmpeg.h"

void KeyframeWorker::run() {
    emit ready(ffmpeg::keyframeTimes(m_path));
}
