#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QCoreApplication>
#include <QGLWidget>
#include <QTime>

#include <QOpenGLShaderProgram>

#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>

#include <memory>

#include "mesh.h"

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
    virtual void keyPressEvent(QKeyEvent* e);

    static void qNormalizeAngle(int &angle);

    void setXTranslation(int x);
    void setYTranslation(int y);
    void setZTranslation(int z);

    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);


private:
    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );

    void createGround();
    void createTeapot();

    std::vector<std::shared_ptr<Mesh> > m_sceneObjects;
    Mesh m_teapot;

    QOpenGLShaderProgram m_pgm;

    QMatrix4x4 m_model;
    QMatrix4x4 m_view;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_mvp;

    QVector3D m_cameraPos;
    QVector3D m_dir;

    int m_mouseDelta;
    int m_xRot;
    int m_yRot;
    int m_zRot;
    int m_xDis;
    int m_yDis;
    int m_zDis;

    QPoint m_lastPos;

};

#endif // GLWIDGET_H
