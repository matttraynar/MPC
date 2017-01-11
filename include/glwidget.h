#ifndef GLWIDGET_H
#define GLWIDGET_H

//Files for Qt
#include <QCoreApplication>
#include <QGLWidget>
#include <QTime>
#include <QOpenGLShaderProgram>
#include <QWheelEvent>
#include <QMouseEvent>

//Other needed includes
#include <memory>
#include <utility>

//And finally the includes for local files
#include "settings.h"
#include "mesh.h"
#include "btworld.h"
#include "btshape.h"

//Inherit this object from a QObject so that it can recieve
//signals and slots
class GLWidget : public QGLWidget
{
 Q_OBJECT

public:
    GLWidget(QWidget* parent = 0 );
    ~GLWidget();

    //Simple function for getting the colour of the object that was last added to the scene
    inline QVector4D getLastAddedColour() const { return m_sceneObjects[m_sceneObjects.size() - 1]->getColour(); }

protected:
    //Declare the OpenGL functions we will overload
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();

    //Functions for user input
    virtual void wheelEvent(QWheelEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);

    //The ticker which will do all the calling to bullet
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

    //A few signals the widget can emit
signals:
    void setSphereNumber(int number);
    void setMeshColour(QVector4D colour);

    //Slots which will be filled by the UI and can then be passed
    //by this widget to various functions and classes
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

    void moveDown();
    void moveUp();
    void stopMove();

private:
    //Method for creating the shader program and loading
    //the view/position matrices
    bool prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath );
    void loadShaderMatrices(float radius);

    //Methods for creating each mesh
    void createGround();
    void createMesh(const char* filepath, const std::string name, QVector3D position);

    //Updates the constraint VAO and VBO so that it reflects any movements they
    //might undergo
    void updateConstraintDrawing();

    //Simple function which checks whether an object with the given name has already
    //been added to the scene
    bool checkExisting(const std::string name, int &position);

    //Containers which contain all the objects in the scene and their positions
    std::vector<std::shared_ptr<Mesh> > m_sceneObjects;
    vector_V m_sceneObjectPositions;

    //Similar to the above containers but for the sphere count of each object and
    //the positions of the spheres
    int_V m_sphereNumbers;
    std::vector< vector_V> m_spherePositions;

    //Containers for storing and drawing the constraint data
    vector_V constraintVerts;
    std::vector< std::vector< std::pair<uint, uint> > >m_constraints;
    QOpenGLVertexArrayObject m_vaoConstraint;
    QOpenGLBuffer m_vboConstraint;

    //Container for storing the radii of the various sphere packs
    std::vector<float> m_sphereRadii;
    Mesh m_sphere = Mesh(QVector4D(1.0f, 0.0f, 0.0f, 1.0f), "sphere");

    //How much to step the simulation by
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

    //Containers which store whether to draw the mesh, spheres and/or constraints
    //for each object in the scene
    std::vector<bool> m_drawMeshStates;
    std::vector<bool> m_drawSphereStates;
    std::vector<bool> m_drawConstraintStates;

    //Container which stores whether each object is being skinned to it's spheres or not
    std::vector<bool> m_meshSkinStates;

    //Boolean which controls whether to be stepping in bullet or not
    bool m_isSimulating;

    //Values used to move objects up and down in the bullet scene
    bool m_moveUp;
    bool m_moveDown;
    QVector3D m_adjustPos;

    //Member used to store the position of the user's mouse click
    QPoint m_lastPos;
};

#endif // GLWIDGET_H
