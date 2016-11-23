#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QCoreApplication>
#include <QGLWidget>
#include <QTime>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>

#include <memory>

#include "assimploader.h"

class GLWidget : public QGLWidget
{
 Q_OBJECT

public:
    GLWidget(QWidget* parent = 0 );
    ~GLWidget();

protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();

    virtual void wheelEvent(QWheelEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);

    static void qNormalizeAngle(int &angle);

    void setXTranslation(int x);
    void setYTranslation(int y);
    void setZTranslation(int z);

    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);


private:
    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );

    std::unique_ptr<AssimpLoader> m_loader;

    QOpenGLShaderProgram m_pgm;
    QOpenGLVertexArrayObject m_vao;

    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

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

    int m_xRot;
    int m_yRot;
    int m_zRot;
    int m_xDis;
    int m_yDis;
    int m_zDis;
    QPoint m_lastPos;

};

#endif // GLWIDGET_H
