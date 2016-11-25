#ifndef GLWIDGET_H
#define GLWIDGET_H

//Files for Qt
#include <QCoreApplication>
#include <QGLWidget>
#include <QTime>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QMouseEvent>

//Other needed includes
#include <memory>

#include "mesh.h"
#include "btworld.h"
#include "btshape.h"

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
    virtual void timerEvent(QTimerEvent* e);

    static void qNormalizeAngle(int &angle);

    void setXTranslation(int x);
    void setYTranslation(int y);
    void setZTranslation(int z);

    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);


private:
    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );
    void loadShaderMatrices();

    void createGround();
    void createTeapot();

    std::vector<std::shared_ptr<Mesh> > m_sceneObjects;
    std::unordered_map< std::string, std::shared_ptr<Mesh> > m_objs;

    std::unique_ptr<BtWorld> m_bullet;

    QOpenGLShaderProgram m_pgm;

    QMatrix4x4 m_model;
    QMatrix4x4 m_view;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_mvp;
    QMatrix4x4 m_trans;

    QVector3D m_position;

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
