#include <QGuiApplication>

#include <QtQuick/QQuickView>

// #include "threadrenderer.h"

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    // qmlRegisterType<ThreadRenderer>("SceneGraphRendering", 1, 0, "Renderer");
    int execReturn = 0;
    QQuickView view;

    // Rendering in a thread introduces a slightly more complicated cleanup
    // so we ensure that no cleanup of graphics resources happen until the
    // application is shutting down.
    view.setPersistentOpenGLContext(true);
    view.setPersistentSceneGraph(true);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///main.qml"));
    view.show();

    execReturn = app.exec();

    return execReturn;
}
