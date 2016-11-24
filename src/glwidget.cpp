#include "glwidget.h"

#include <iostream>

GLWidget::GLWidget( QWidget* parent )
    : QGLWidget(parent ),
    m_loader(new AssimpLoader)
{
    //Setup the format for the window
    QSurfaceFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QSurfaceFormat::CoreProfile ); // Requires >=Qt-4.8.0

    //Enable a few flags for rendering later
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    //Initialise variables for mouse control to 0
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
    m_zDis = 0;
    m_mouseDelta = 0;
    m_cameraPos = QVector3D(5,5,5);
}

GLWidget::~GLWidget()
{

}

void GLWidget::initializeGL()
{
    //Set the background colour for the window
     glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );

     //Create the correct shader program and if it fails immediately return
     if ( !prepareShaderProgram( "shaders/vert.glsl", "shaders/frag.glsl" ) )
     {
         exit(1);
     }

     //Set an initial view matrix
     m_view.setToIdentity();
     m_view.lookAt(QVector3D(0.0f, 0.0f, 1.2f),    // Camera Position
                   QVector3D(0.0f, 0.0f, 0.0f),    // Point camera looks towards
                   QVector3D(0.0f, 1.0f, 0.0f));   // Up vector

     //Load a mesh using the assimp loader class
     m_loader->loadMesh("objFiles/teapot.obj");


     //Bind the shader program to the context
     if ( !m_pgm.bind() )
     {
         qWarning() << "Could not bind shader program to context";
         return;
     }

     //Create the VAO if it hasn't been created already and then bind it to the current context
     if(!m_vao.isCreated())
     {
         m_vao.create();
     }

     m_vao.bind();

     //Create a VBO to store new data in
     m_vbo.create();
     m_vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );

     //Bind the VBO to the context, if it fails print the warning and return
     if ( !m_vbo.bind() )
     {
         qWarning() << "Could not bind vertex buffer to the context";
         return;
     }
     //Allocate data to the VBO
     m_vbo.allocate( &m_loader->m_verts[0], m_loader->m_verts.size() * sizeof(float) * 3);

     //Pass the vbo data into the shader program
     m_pgm.enableAttributeArray("vertexPos");
     m_pgm.setAttributeArray("vertexPos", GL_FLOAT, 0, 3);

     //Release the vertex buffer object
     m_vbo.release();

     //Create a buffer object for the normal data
     m_nbo.create();
     m_nbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
     m_nbo.bind();

     //Allocate the data
     m_nbo.allocate(&m_loader->m_norms[0], m_loader->m_norms.size() * sizeof(float) * 3);

     //Pass the data into the shader program
     m_pgm.enableAttributeArray("vertexNorm");
     m_pgm.setAttributeArray("vertexNorm", GL_FLOAT, 0, 3);

     //Release the normals buffer object
     m_nbo.release();

     //Create a final buffer object for the obj indices
     m_ibo.create();
     m_ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
     m_ibo.bind();

     //Allocate the data
     m_ibo.allocate(&m_loader->m_meshIndex[0], m_loader->m_meshIndex.size() * sizeof(uint));

     //Release the index buffer and finally the VAO
     m_vao.release();

     //Then release the shader program

     m_pgm.release();
}

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
    glEnable(GL_DEPTH_TEST);

    //Now bind the shader program to the current context
    m_pgm.bind();


    //Bind the vao to the context
    m_vao.bind();

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
    m_model.translate(QVector3D(0, 0, -0.1f*m_zDis));
    m_model.rotate(m_xRot / 16.0f, QVector3D(1, 0, 0));
    m_model.rotate(m_yRot / 16.0f, QVector3D(0, 1, 0));

    //Calculate the MVP
    m_mvp = m_proj * m_view * m_model;


    //Pass the MVP into the shader
    m_pgm.setUniformValue("M",m_model);
    m_pgm.setUniformValue("MVP",m_mvp);

    QVector4D colour(0.9,0.9,0.9,1.0);
    m_pgm.setUniformValue("mCol", colour);

    // Draw stuff
//    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDrawElements(GL_TRIANGLES, m_loader->m_meshIndex.size(), GL_UNSIGNED_INT, &m_loader->m_meshIndex[0]);

    //Release the VAO and the shader program
    m_vao.release();
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
    else if (e->buttons() & Qt::RightButton)
    {
        setZTranslation(m_zDis + dy);
    }

    //Update the last position
    m_lastPos = e->pos();
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


