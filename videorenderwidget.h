#ifndef QYUVOPENGLWIDGET_H
#define QYUVOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class MainWindowObserver;

class VideoRenderWidget
    : public QOpenGLWidget
    , protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit VideoRenderWidget(QWidget *parent = Q_NULLPTR);
    virtual ~VideoRenderWidget() Q_DECL_OVERRIDE;

    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    QSize sizeHint() const Q_DECL_OVERRIDE;
    void setFrameSize(const QSize &frameSize);
    const QSize &frameSize();
    void updateTextures(const quint8 *dataY, const quint8 *dataU, const quint8 *dataV, quint32 linesizeY,
                        quint32 linesizeU, quint32 linesizeV);
public slots:
    void updateBuffer(QSize size, const quint8 *dataY, const quint8 *dataU, const quint8 *dataV,
                      quint32 linesizeY, quint32 linesizeU, quint32 linesizeV);

protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;

private:
    void initShader();
    void initTextures();
    void deInitTextures();
    void updateTexture(GLuint texture, quint32 textureType, const quint8 *pixels, quint32 stride);

private:
    bool m_needUpdate = false;
    bool m_textureInited = false;
    // YUV纹理，用于生成纹理贴图
    GLuint m_texture[3] = {0};
    GLuint m_progYuv;
    // 视频帧尺寸
    QSize m_frameSize = {-1, -1};
    // 顶点缓冲对象(Vertex Buffer Objects, VBO)：默认即为VertexBuffer(GL_ARRAY_BUFFER)类型
    MainWindowObserver *m_observer = Q_NULLPTR;
    QOpenGLBuffer m_vbo;
    // 着色器程序：编译链接着色器
    QOpenGLShaderProgram m_shaderProgram;
};

#endif // QYUVOPENGLWIDGET_H
