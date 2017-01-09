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

#include "settings.h"
#include "mesh.h"
#include "btworld.h"
#include "btshape.h"

class GLWidget : public QGLWidget
{
 Q_OBJECT

public:
    GLWidget(QWidget* parent = 0 );
    ~GLWidget();

    inline QVector4D getLastAddedColour() const { return m_sceneObjects[m_sceneObjects.size() - 1]->getColour(); }

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

signals:
    void setSphereNumber(int number);
    void setMeshColour(QVector4D colour);

public slots:
    void setWorldColour(QColor colour);
    void setPlaneColour(QColor colour);

    void addNewMesh(QString fileName, QString meshName, QVector3D position);
    void removeMesh(QString meshName);

    void setMeshShader(QString meshName, ShaderSettings &settings);

    void runDistanceField(QString meshName, DistanceFieldSettings &settings);
    void runSpherePack(QString meshName, SpherePackSettings &settings);
    void runConstraints(QString meshName, ConstraintSettings &settings);

    void toggleDrawSpheres(QString meshName, bool drawSpheres);
    void toggleDrawConstraints(QString meshName, bool drawConstraints);
    void toggleSimulation(bool isSimulating);

    void resetSimulation();
    void setSimulation(float stepValue);

private:
    //Method for creating the shader program and loading
    //the view/position matrices
    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );
    void loadShaderMatrices(float radius);

    //Methods for creating each mesh
    void createGround();
    void createMesh(const char* filepath, const std::string name, QVector3D position);

    void updateConstraintDrawing();

    bool checkExisting(const std::string name, int &position);

    //Container which contains all the objects in the scene
    std::vector<std::shared_ptr<Mesh> > m_sceneObjects;
    std::vector<QVector3D> m_sceneObjectPositions;
    int_V m_sphereNumbers;
    std::vector< vector_V> m_spherePositions;

    vector_V constraintVerts;
    std::vector< std::vector< std::pair<uint, uint> > >m_constraints;
    QOpenGLVertexArrayObject m_vaoConstraint;
    QOpenGLBuffer m_vboConstraint;

    std::vector<float> m_sphereRadii;
    float m_radius;

    float m_simulationStep;

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

    std::vector<bool> m_drawMeshStates;
    std::vector<bool> m_drawSphereStates;
    std::vector<bool> m_drawConstraintStates;

    std::vector<bool> m_meshSkinStates;

    bool m_isSimulating;
    bool m_drawMesh;
    bool m_drawSpheres;

    bool m_moveUp;
    bool m_moveDown;
    bool m_adjust;
    QVector3D m_adjustPos;

    //Member used to store the position of the user's mouse click
    QPoint m_lastPos;
};

#endif // GLWIDGET_H
