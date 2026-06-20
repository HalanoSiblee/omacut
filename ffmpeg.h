#pragma once

#include <QImage>
#include <QString>
#include <QStringList>
#include <QVector>

// Thin wrappers around the ffmpeg/ffprobe command-line tools.
namespace ffmpeg {

struct VideoInfo {
    QString path;
    double duration = 0.0;  // seconds
    int width = 0;
    int height = 0;
    double fps = 0.0;
    bool ok = false;
    QString error;
};

// Probe a file for duration, dimensions and frame rate (runs ffprobe).
VideoInfo probe(const QString &path);

// Timestamps (seconds, ascending) of every video keyframe. A stream-copy cut
// can only begin on one of these, so this is what decides copy vs re-encode.
// Runs ffprobe over packets (no decoding); may be slow for very long files.
QVector<double> keyframeTimes(const QString &path);

// Grab a single frame at `time` seconds, scaled to `height` px.
// Returns a null QImage on failure.
QImage thumbnail(const QString &path, double time, int height = 90);

// Build the ffmpeg argument list that writes [start, end] of src to dst.
// When reencode is true the cut is frame-accurate (libx264/aac); otherwise
// streams are copied for an instant, lossless, keyframe-aligned cut.
QStringList trimArgs(const QString &src, const QString &dst,
                     double start, double end, bool reencode);

// Locate a tool on PATH; returns empty string if missing.
QString toolPath(const QString &tool);

}  // namespace ffmpeg
