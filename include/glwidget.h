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
#include <utility>

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
    //Declare the OpenGL functions we will overload
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();

    virtual void wheelEvent(QWheelEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void timerEvent(QTimerEvent* e);

    //A quick function we'll use for camera movement
    static void qNormalizeAngle(int &angle);

    //Some more functions called when the camera is 'moved'
    void setXTranslation(int x);
    void setYTranslation(int y);
    void setZTranslation(int z);

    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);


private:
    //Method for creating the shader program and loading
    //the view/position matrices
    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );
    void loadShaderMatrices(float radius);

    //Methods for creating each mesh
    void createGround();
    void createMesh(const char* filepath, const std::string name, QVector3D position);

    bool checkExisting(const std::string name, int &position);

    //Container which contains all the objects in the scene
    std::vector<std::shared_ptr<Mesh> > m_sceneObjects;
    std::vector<QVector3D> m_sceneObjectPositions;
    int_V m_sphereNumbers;
    float m_radius;

    //An unordered map used to refer to each mesh by a name
    std::unordered_map< std::string, std::shared_ptr<Mesh> > m_objs;

    //An instance of the bullet world
    std::unique_ptr<BtWorld> m_bullet;

    //The shader program we'll use
    QOpenGLShaderProgram m_pgm;

    //Matrices used for viewing the world
    QMatrix4x4 m_model;
    QMatrix4x4 m_view;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_mvp;

    //Positional vector used to draw things in different places
    QVector3D m_position;

    //Vectors for storing camera data
    QVector3D m_cameraPos;
    QVector3D m_dir;

    //Values editted when the camera is moved
    int m_mouseDelta;
    int m_xRot;
    int m_yRot;
    int m_zRot;
    int m_xDis;
    int m_yDis;
    int m_zDis;

    bool m_isSimulating;
    bool m_drawMesh;
    bool m_drawSpheres;
    bool m_plastic;

    bool m_moveUp;
    bool m_moveDown;
    bool m_adjust;
    QVector3D m_adjustPos;

    //Member used to store the position of the user's mouse click
    QPoint m_lastPos;
};

#endif // GLWIDGET_H
