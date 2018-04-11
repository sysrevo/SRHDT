#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>

#include "BackEnd.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<BackEnd>("data.backend", 1, 0, "BackEnd");
    BackEnd backend;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("backend", &backend);
    engine.addImageProvider(QLatin1String("colors"), backend.getProvider());

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
