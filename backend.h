#pragma once

#include <QObject>
#include <QString>
#include <QUrl>

#include "ffmpeg.h"

class ThumbProvider;

// The bridge between QML and the ffmpeg/ffprobe layer. Holds the currently
// loaded video's info and drives thumbnail generation and export.
class Backend : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source NOTIFY infoChanged)
    Q_PROPERTY(double duration READ duration NOTIFY infoChanged)
    Q_PROPERTY(int videoWidth READ videoWidth NOTIFY infoChanged)
    Q_PROPERTY(int videoHeight READ videoHeight NOTIFY infoChanged)
    Q_PROPERTY(int thumbCount READ thumbCount NOTIFY thumbsChanged)
    Q_PROPERTY(int thumbRevision READ thumbRevision NOTIFY thumbsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    // Bumps whenever the keyframe analysis changes; reference it in QML
    // bindings so willReencode() re-evaluates once analysis is ready.
    Q_PROPERTY(int analysisRevision READ analysisRevision NOTIFY analysisChanged)

public:
    explicit Backend(ThumbProvider *provider, QObject *parent = nullptr);

    QUrl source() const { return m_source; }
    double duration() const { return m_info.duration; }
    int videoWidth() const { return m_info.width; }
    int videoHeight() const { return m_info.height; }
    int thumbCount() const { return m_thumbCount; }
    int thumbRevision() const { return m_thumbRevision; }
    bool busy() const { return m_busy; }
    QString status() const { return m_status; }
    int analysisRevision() const { return m_analysisRevision; }

    // Load a video (probes it, then kicks off thumbnail + keyframe analysis).
    Q_INVOKABLE bool load(const QUrl &url);

    // The folder containing the loaded video, for the save dialog.
    Q_INVOKABLE QUrl sourceFolder() const;

    // Suggested "<name>_trimmed.<ext>" target next to the source.
    Q_INVOKABLE QUrl suggestedExportUrl() const;

    // True if a cut starting at `start` would need re-encoding to be accurate
    // (i.e. the start does not land on a keyframe). This is the auto decision.
    Q_INVOKABLE bool willReencode(double start) const;

    // Write [start, end] (seconds) of the loaded video to dst, choosing copy
    // vs re-encode automatically via willReencode().
    Q_INVOKABLE void exportClip(const QUrl &dst, double start, double end);

signals:
    void infoChanged();
    void thumbsChanged();
    void busyChanged();
    void statusChanged();
    void analysisChanged();
    void exportDone(const QString &path);
    void exportFailed(const QString &message);
    void loadError(const QString &message);

private:
    void setBusy(bool busy);
    void setStatus(const QString &status);
    void startThumbs();
    void startKeyframes();
    double alignEpsilon() const;

    ThumbProvider *m_provider;
    ffmpeg::VideoInfo m_info;
    QString m_path;
    QUrl m_source;
    int m_thumbCount = 0;
    int m_thumbRevision = 0;
    int m_analysisRevision = 0;
    bool m_busy = false;
    QString m_status;
    QVector<double> m_keyframes;
};
