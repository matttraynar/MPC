#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QCoreApplication>
#include <QGLWidget>
#include <QTime>

#include <QGLBuffer>
#include <QGLShaderProgram>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>

class GLWidget : public QGLWidget
{
 Q_OBJECT

public:
    GLWidget(QWidget* parent = 0 );

protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();

    virtual void keyPressEvent( QKeyEvent* e );
    virtual void wheelEvent(QWheelEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);

private:
    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );

    QGLShaderProgram m_pgm;
    QGLBuffer m_vbo;

    QMatrix4x4 m_model;
    QMatrix4x4 m_view;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_mvp;

    int m_mouseDelta;
    QVector3D m_cameraPos;

    double m_lastTime;
    bool m_mousePress;
    int m_xPos;
    int m_yPos;
    float m_hAngle;
    float m_vAngle;
    QVector3D m_dir;
    QVector3D m_right;
    QVector3D m_up;

};

#endif // GLWIDGET_H
