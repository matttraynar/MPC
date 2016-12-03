#include "glwidget.h"

#include <iostream>

GLWidget::GLWidget( QWidget* parent )
    : QGLWidget(parent )
{
    //Setup the format for the window
    QSurfaceFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QSurfaceFormat::CoreProfile ); // Requires >=Qt-4.8.0

    m_bullet.reset(new BtWorld());
    m_bullet->setGravity(0, -9.8, 0);
    m_bullet->addGround();

    //Initialise variables for mouse control to 0
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
    m_zDis = 0;
    m_mouseDelta = 0;
    m_cameraPos = QVector3D(5,5,5);
    m_trans = QMatrix4x4();
    m_position = QVector3D();
}

GLWidget::~GLWidget()
{

}

void GLWidget::initializeGL()
{
    //Set the background colour for the window
     glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );

     //Enable depth testing for rendering later
     glEnable(GL_DEPTH_TEST);

     //Create the correct shader program and if it fails immediately return
     if ( !prepareShaderProgram( "shaders/vert.glsl", "shaders/frag.glsl" ) )
     {
         exit(1);
     }

     m_pgm.bind();

     //Set an initial view matrix
     m_view.setToIdentity();
     m_view.lookAt(QVector3D(0.0f, 0.0f, 1.2f),    // Camera Position
                   QVector3D(0.0f, 0.0f, 0.0f),    // Point camera looks towards
                   QVector3D(0.0f, 1.0f, 0.0f));   // Up vector


//     createGround();

//     createTeapot();

     startTimer(10);

     BtShape* shapes = BtShape::instance();
     shapes->addMesh("groundPlane","objFiles/ground.obj",QVector3D(1.0,1.0,1.0));
     shapes->addMesh("teapot","objFiles/teapotLARGE.obj",QVector3D(0.0,0.0,1.0));

     m_bullet->addMesh("groundPlane",QVector3D(0,0,0));
     createGround();

     m_bullet->addMesh("teapot",QVector3D(0,10,0));
     createTeapot();

     m_pgm.release();
}

//----------------------------------------------------- PRIVATE METHODS -----------------------------------------------------//

bool GLWidget::prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath )
{
     // Load the vertex shader
     bool result = m_pgm.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShaderPath );

     //Check if the function succeeded, if not print why and return
     if ( !result )
     {
         qWarning() << m_pgm.log();
         return result;
     }

    //Do the same with the fragment shader
     result = m_pgm.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShaderPath );

     if ( !result )
     {
        qWarning() << m_pgm.log();
        return result;
     }

    //Finally link the shaders to the program
     result = m_pgm.link();

     if ( !result )
     {
        qWarning() << m_pgm.log();
        return result;
     }

     return result;
}

void GLWidget::loadShaderMatrices()
{
    //Get vector from camera to the origin
    QVector3D o(0,0,0);
    QVector3D camToOrig = o - m_cameraPos;

    //Normalize and calculate a new position somewhere along this vector
    camToOrig.normalize();
    QVector3D newCamPos = m_cameraPos + (m_mouseDelta * camToOrig);

    //Reset the view matrix and set to look at origin from the new position
    m_view.setToIdentity();
    m_view.lookAt(newCamPos + m_dir, QVector3D(0,1,0), QVector3D(0,1,0));

    //Reset the projection matrix and set to the right perspective
    m_proj.setToIdentity();
    m_proj.perspective(70.0f, float(width())/height(), 0.1f, 1000.0f);

    //Reset the model matrix and set to the right matrix taking into account mouse movement
    m_model.setToIdentity();

    m_model.rotate(m_xRot / 16.0f, QVector3D(1, 0, 0));
    m_model.rotate(m_yRot / 16.0f, QVector3D(0, 1, 0));

    m_model.translate(m_position);
    m_model.translate(0.0f, -1.0f, 0.0f);

    //Calculate the MVP
    m_mvp = m_proj * m_view * m_model;

    //Pass the MVP into the shader
    m_pgm.setUniformValue("M",m_model);
    m_pgm.setUniformValue("MVP",m_mvp);
}

void GLWidget::createGround()
{
     //Bind the shader program to the context
    m_pgm.bind();

    //Create a new mesh shared pointer and use the colour constructor
    std::shared_ptr<Mesh> ground(new Mesh(QVector4D(1.0,1.0,1.0,1.0)));

    //Load the ground plane obj
    ground->loadMesh("objFiles/ground.obj");

    //Load the neccesary vaos and vbos
    ground->prepareMesh(m_pgm);

    //Add the pointer to the vector of scene objects
    m_sceneObjects.push_back(ground);
//    m_objs["groundPlane"].reset(ground);

    //Release the shader program
    m_pgm.release();
}

void GLWidget::createTeapot()
{
    //Bind the shader program to the context
    m_pgm.bind();

    //Create a new mesh shared pointer and use the colour constructor
    std::shared_ptr<Mesh> teapot(new Mesh(QVector4D(0.9,1.0,1.0,1.0)));

    //Load the teapot obj
    teapot->loadMesh("objFiles/teapotLARGE.obj");
    teapot->packSpheres();

    //Load the neccesary vaos and vbos
    teapot->prepareMesh(m_pgm);

    //Add the pointer to the vector of scene objects
    m_sceneObjects.push_back(teapot);

    //Release the shader program
    m_pgm.release();
}

//------------------------------------------------- RESIZE AND PAINT WINDOW -------------------------------------------------//

void GLWidget::resizeGL( int w, int h )
{
    // Set the viewport to window dimensions
    glViewport( 0, 0, w, h );

    //Update the projection matrix and re-render the window
    m_proj.setToIdentity();
    m_proj.perspective(70.0f, float(w)/h, 0.1f, 1000.0f);
    update();
}

void GLWidget::paintGL()
{
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //Now bind the shader program to the current context
    m_pgm.bind();

    loadShaderMatrices();

    //Draw mesh
    Mesh sphere;
    sphere.loadMesh("objFiles/sphere.obj");
    sphere.prepareMesh(m_pgm);
    std::vector<QVector3D> verts = sphere.getVerts();

    float radius = 0;
    float length = 0;

    for(uint i = 0; i < verts.size(); ++i)
    {
        length = (pow(verts[i][0],2) + pow(verts[i][1],2) + pow(verts[i][2],2));

        radius += sqrt(length);
    }

    radius /= verts.size();

    std::cout<<"Radius: "<<radius<<'\n';


    uint nBodies = m_bullet->getNumCollisionObjects();

    for(uint i = 0; i < nBodies; ++i)
    {
        m_position = m_bullet->getTransform(i);

        loadShaderMatrices();

        std::string name = m_bullet->getBodyNameAt(i);

        if(name == "teapot")
        {
            m_pgm.setUniformValue("mCol",m_sceneObjects[0]->m_colour);
            m_sceneObjects[1]->draw();

            for(uint j = 0; j < m_sceneObjects[1]->m_spherePositions.size(); ++j)
            {
                m_position = m_sceneObjects[1]->m_spherePositions[j];

                loadShaderMatrices();

                sphere.draw();
            }
        }
    }

    m_position = QVector3D(0.0f, 0.0f, 0.0f);
    loadShaderMatrices();

    m_pgm.setUniformValue("mCol",m_sceneObjects[1]->m_colour);
    m_sceneObjects[0]->draw();

    m_pgm.release();
}

//------------------------------------------- MOUSE/SCREEN INTERACTION FUNCTIONS --------------------------------------------//

void GLWidget::wheelEvent(QWheelEvent *e)
{
    //Get the number of degrees the wheel has turned
    int numDegrees = e->delta() / 8;
    int numSteps = numDegrees / 15;

    //Add the steps to a member variable used in paintGL
    m_mouseDelta += numSteps;

    update();
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    //Store the position the mouse was pressed in
    m_lastPos = e->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    //Get the change in x & y since the last click
    int dx = e->x() - m_lastPos.x();
    int dy = e->y() - m_lastPos.y();

    //See if the click was a left or a right click, change position accordingly
    if (e->buttons() & Qt::LeftButton)
    {
        setXRotation(m_xRot + 8 * dy);
        setYRotation(m_yRot + 8 * dx);
    }

    //Update the last position
    m_lastPos = e->pos();
    update();
}

void GLWidget::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_W:
        for(uint i = 0; i < m_sceneObjects.size(); ++i)
        {
            m_sceneObjects[i]->setWireMode();
        }

//        m_teapot.setWireMode();

        break;
    case Qt::Key_T:
        m_bullet->addMesh("teapot",QVector3D(0,10,0));

        m_pgm.bind();
        createTeapot();
        m_pgm.release();

        break;

    case Qt::Key_Escape:
        close();
        break;

    default:
        break;
    }

    update();
}

void GLWidget::qNormalizeAngle(int &angle)
{
    //Make sure the angle is adjusted accordingly
    while (angle < 0) angle += 360 * 16;
    while (angle > 360 * 16) angle -= 360 * 16;
}

void GLWidget::setXTranslation(int x)
{
    //Set the new x position and call update on the window
    qNormalizeAngle(x);

    if (x != m_xDis)
    {
        m_xDis = x;
        update();
    }
}

void GLWidget::setYTranslation(int y)
{
    //Set the new y position and call update on the window
    qNormalizeAngle(y);

    if (y != m_yDis)
    {
        m_yDis = y;
        update();
    }
}

void GLWidget::setZTranslation(int z)
{
    //Set the new z position and call update on the window
    qNormalizeAngle(z);

    if (z != m_zDis)
    {
        m_zDis= z;
        update();
    }
}

void GLWidget::setXRotation(int angle)
{
    //Set the new x rotation and call update on the window

    qNormalizeAngle(angle);

    if (angle != m_xRot)
    {
        m_xRot = angle;
        update();
    }
}

void GLWidget::setYRotation(int angle)
{
    //Set the new y rotation and call update on the window
    qNormalizeAngle(angle);

    if (angle != m_yRot)
    {
        m_yRot = angle;
        update();
    }
}

void GLWidget::setZRotation(int angle)
{
    //Set the new z rotation and call update on the window
    qNormalizeAngle(angle);

    if (angle != m_zRot)
    {
        m_zRot = angle;
        update();
    }
}

void GLWidget::timerEvent(QTimerEvent *e)
{
    m_bullet->step(1.0f/60.0f, 10);
    update();
}
