#ifndef RENDERHELPER_H
#define RENDERHELPER_H

#include <QtCore/QMutex>
#include <QtCore/QThread>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFramebufferObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QtGui/QGuiApplication>
#include <QtGui/QOffscreenSurface>

#include <QtQuick/QQuickWindow>
#include <qsgsimpletexturenode.h>

class BeanCellItemPrivate;

/*
 * The render thread shares a context with the scene graph and will
 * render into two separate FBOs, one to use for display and one
 * to use for rendering
 */
class RenderThread : public QThread
{
    Q_OBJECT
public:
    RenderThread(const QSize &size, BeanCellItemPrivate* renderer);
    QOffscreenSurface *surface;
    QOpenGLContext *context;

public slots:
    void renderNext();

    void shutDown();
    void setOffset(const QPointF &offset);

signals:
    void textureReady(int id, const QSize &size);

private:
    QOpenGLFramebufferObject* m_renderFbo;
    QOpenGLFramebufferObject* m_displayFbo;
    BeanCellItemPrivate* m_renderer;
    QSize m_size;
    QPointF m_offset;
};



class TextureNode : public QObject, public QSGSimpleTextureNode
{
    Q_OBJECT

public:
    TextureNode(QQuickWindow *window);

    ~TextureNode();

signals:
    void textureInUse();
    void pendingNewTexture();

public slots:

    // This function gets called on the FBO rendering thread and will store the
    // texture id and size and schedule an update on the window.
    void newTexture(int id, const QSize &size);


    // Before the scene graph starts to render, we update to the pending texture
    void prepareNode();
private:

    int m_id;
    QSize m_size;

    QMutex m_mutex;

    QSGTexture *m_texture;
    QQuickWindow *m_window;
};

#endif // RENDERHELPER_H
