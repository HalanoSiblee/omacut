#include "ffmpeg.h"

#include <algorithm>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>

namespace ffmpeg {

QString toolPath(const QString &tool) {
    return QStandardPaths::findExecutable(tool);
}

VideoInfo probe(const QString &path) {
    VideoInfo info;
    info.path = path;

    const QString ffprobe = toolPath("ffprobe");
    if (ffprobe.isEmpty()) {
        info.error = "`ffprobe` was not found on your PATH. Install ffmpeg.";
        return info;
    }

    QProcess proc;
    proc.start(ffprobe, {
        "-v", "error",
        "-print_format", "json",
        "-show_format",
        "-show_streams",
        "-select_streams", "v:0",
        path,
    });
    proc.waitForFinished(-1);

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
        info.error = QString::fromUtf8(proc.readAllStandardError()).trimmed();
        if (info.error.isEmpty())
            info.error = "ffprobe failed.";
        return info;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(proc.readAllStandardOutput());
    const QJsonObject root = doc.object();
    const QJsonArray streams = root.value("streams").toArray();
    if (streams.isEmpty()) {
        info.error = "No video stream found in this file.";
        return info;
    }

    const QJsonObject stream = streams.first().toObject();

    // Duration can live on the stream or on the container.
    QString durationStr = stream.value("duration").toString();
    if (durationStr.isEmpty())
        durationStr = root.value("format").toObject().value("duration").toString();
    if (durationStr.isEmpty()) {
        info.error = "Could not determine the video duration.";
        return info;
    }

    info.duration = durationStr.toDouble();
    info.width = stream.value("width").toInt();
    info.height = stream.value("height").toInt();

    // r_frame_rate comes as "num/den", e.g. "30000/1001".
    const QString rate = stream.value("r_frame_rate").toString();
    const int slash = rate.indexOf('/');
    if (slash > 0) {
        const double num = rate.left(slash).toDouble();
        const double den = rate.mid(slash + 1).toDouble();
        if (den > 0.0)
            info.fps = num / den;
    }

    info.ok = info.duration > 0.0;
    if (!info.ok)
        info.error = "Video has a zero or invalid duration.";
    return info;
}

QVector<double> keyframeTimes(const QString &path) {
    QVector<double> times;
    const QString ffprobe = toolPath("ffprobe");
    if (ffprobe.isEmpty())
        return times;

    // Read packet timestamps + flags without decoding; 'K' marks a keyframe.
    QProcess proc;
    proc.start(ffprobe, {
        "-v", "error",
        "-select_streams", "v:0",
        "-show_entries", "packet=pts_time,flags",
        "-of", "csv=p=0",
        path,
    });
    proc.waitForFinished(-1);
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0)
        return times;

    const QByteArray out = proc.readAllStandardOutput();
    for (const QByteArray &line : out.split('\n')) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;
        const int comma = trimmed.indexOf(',');
        if (comma <= 0)
            continue;
        const QByteArray timeStr = trimmed.left(comma);
        const QByteArray flags = trimmed.mid(comma + 1);
        if (!flags.contains('K'))
            continue;
        bool okNum = false;
        const double t = timeStr.toDouble(&okNum);
        if (okNum)
            times.append(t);
    }
    std::sort(times.begin(), times.end());
    return times;
}

QImage thumbnail(const QString &path, double time, int height) {
    const QString ffmpeg = toolPath("ffmpeg");
    if (ffmpeg.isEmpty())
        return {};

    QProcess proc;
    proc.start(ffmpeg, {
        "-loglevel", "error",
        "-ss", QString::number(qMax(time, 0.0), 'f', 3),
        "-i", path,
        "-frames:v", "1",
        "-vf", QString("scale=-1:%1").arg(height),
        "-f", "image2pipe",
        "-vcodec", "mjpeg",
        "pipe:1",
    });
    proc.waitForFinished(-1);

    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0)
        return {};

    const QByteArray data = proc.readAllStandardOutput();
    QImage img;
    img.loadFromData(data, "JPEG");
    return img;
}

QStringList trimArgs(const QString &src, const QString &dst,
                     double start, double end, bool reencode) {
    QStringList args = {"-y", "-loglevel", "error"};
    // -ss before -i seeks fast; -t gives the duration to copy.
    args << "-ss" << QString::number(start, 'f', 3)
         << "-i" << src
         << "-t" << QString::number(qMax(end - start, 0.0), 'f', 3);
    if (reencode) {
        args << "-c:v" << "libx264" << "-preset" << "veryfast"
             << "-crf" << "18" << "-c:a" << "aac";
    } else {
        args << "-c" << "copy";
    }
    args << dst;
    return args;
}

}  // namespace ffmpeg
