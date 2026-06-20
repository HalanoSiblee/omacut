#include "backend.h"

#include <algorithm>

#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include "keyframeworker.h"
#include "thumbprovider.h"
#include "thumbworker.h"

namespace {
constexpr int kThumbCount = 12;
}

Backend::Backend(ThumbProvider *provider, QObject *parent)
    : QObject(parent), m_provider(provider) {}

void Backend::setBusy(bool busy) {
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void Backend::setStatus(const QString &status) {
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}

bool Backend::load(const QUrl &url) {
    const QString path = url.toLocalFile();
    const ffmpeg::VideoInfo info = ffmpeg::probe(path);
    if (!info.ok) {
        emit loadError(info.error);
        return false;
    }

    m_info = info;
    m_path = path;
    m_source = url;

    // New video: drop the old filmstrip and bump the revision so QML reloads.
    m_thumbCount = 0;
    ++m_thumbRevision;
    m_provider->setImages({});
    emit thumbsChanged();

    // Reset keyframe analysis; default to "needs re-encode" until it arrives.
    m_keyframes.clear();
    ++m_analysisRevision;
    emit analysisChanged();

    emit infoChanged();

    setStatus(QString("%1×%2 · generating thumbnails…")
                  .arg(info.width)
                  .arg(info.height));
    startThumbs();
    startKeyframes();
    return true;
}

void Backend::startThumbs() {
    auto *worker = new ThumbWorker(m_path, m_info.duration, kThumbCount, this);
    connect(worker, &ThumbWorker::ready, this, [this, worker](const QVector<QImage> &images) {
        m_provider->setImages(images);
        m_thumbCount = images.size();
        ++m_thumbRevision;
        emit thumbsChanged();
        setStatus(QString());
        worker->deleteLater();
    });
    connect(worker, &ThumbWorker::finished, worker, [worker] {
        // Safety net if `ready` was never delivered.
        if (worker->parent())
            worker->deleteLater();
    });
    worker->start();
}

void Backend::startKeyframes() {
    auto *worker = new KeyframeWorker(m_path, this);
    connect(worker, &KeyframeWorker::ready, this, [this, worker](const QVector<double> &times) {
        m_keyframes = times;
        ++m_analysisRevision;
        emit analysisChanged();
        worker->deleteLater();
    });
    connect(worker, &KeyframeWorker::finished, worker, [worker] {
        if (worker->parent())
            worker->deleteLater();
    });
    worker->start();
}

double Backend::alignEpsilon() const {
    // Treat the start as "on a keyframe" if within half a frame of one.
    if (m_info.fps > 0.0)
        return 0.5 / m_info.fps;
    return 0.02;
}

bool Backend::willReencode(double start) const {
    const double eps = alignEpsilon();
    // The very start of the file is always a keyframe.
    if (start <= eps)
        return false;
    // No analysis yet: be safe and assume an exact (re-encoded) cut is needed.
    if (m_keyframes.isEmpty())
        return true;
    // Find the last keyframe at or before `start`; copy is exact only if it
    // sits essentially on that keyframe.
    auto it = std::upper_bound(m_keyframes.begin(), m_keyframes.end(), start + eps);
    if (it == m_keyframes.begin())
        return true;
    const double kf = *(it - 1);
    return std::abs(start - kf) > eps;
}

QUrl Backend::sourceFolder() const {
    if (m_path.isEmpty())
        return {};
    return QUrl::fromLocalFile(QFileInfo(m_path).absolutePath());
}

QUrl Backend::suggestedExportUrl() const {
    if (m_path.isEmpty())
        return {};
    const QFileInfo src(m_path);
    const QString target =
        src.dir().filePath(src.completeBaseName() + "_trimmed." + src.suffix());
    return QUrl::fromLocalFile(target);
}

void Backend::exportClip(const QUrl &dst, double start, double end) {
    if (m_path.isEmpty() || !m_info.ok)
        return;

    const QString outPath = dst.toLocalFile();
    const QString ffmpegBin = ffmpeg::toolPath("ffmpeg");
    if (ffmpegBin.isEmpty()) {
        emit exportFailed("`ffmpeg` was not found on your PATH.");
        return;
    }

    const bool reencode = willReencode(start);
    setBusy(true);
    setStatus(reencode ? "Exporting (precise re-encode)…" : "Exporting (lossless)…");

    const QStringList args = ffmpeg::trimArgs(m_path, outPath, start, end, reencode);

    auto *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(proc, &QProcess::finished, this,
            [this, proc, outPath](int code, QProcess::ExitStatus exitStatus) {
                const QString err = QString::fromUtf8(proc->readAll()).trimmed();
                proc->deleteLater();
                setBusy(false);
                if (exitStatus == QProcess::NormalExit && code == 0) {
                    setStatus(QString("Saved %1").arg(outPath));
                    emit exportDone(outPath);
                } else {
                    setStatus(QString());
                    emit exportFailed(err.isEmpty() ? "ffmpeg trim failed." : err);
                }
            });
    proc->start(ffmpegBin, args);
}
