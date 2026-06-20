// omacut — a dead-simple video length trimmer. Qt Quick (QML) UI, ffmpeg cuts.

#include <QGuiApplication>
#include <QImage>
#include <QMetaType>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>
#include <QVector>

#include "backend.h"
#include "thumbprovider.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("omacut");

    // Modern, themeable controls (the same family Quickshell builds on).
    QQuickStyle::setStyle("Material");

    // Lets the worker threads deliver results across threads.
    qRegisterMetaType<QVector<QImage>>("QVector<QImage>");
    qRegisterMetaType<QVector<double>>("QVector<double>");

    QQmlApplicationEngine engine;

    // The engine takes ownership of the image provider.
    auto *provider = new ThumbProvider();
    engine.addImageProvider("thumbs", provider);

    Backend backend(provider, &app);
    engine.rootContext()->setContextProperty("backend", &backend);

    engine.load(QUrl("qrc:/Main.qml"));
    if (engine.rootObjects().isEmpty())
        return -1;

    // Optionally open a file passed on the command line.
    const QStringList args = app.arguments();
    if (args.size() > 1)
        backend.load(QUrl::fromLocalFile(args.at(1)));

    return app.exec();
}
