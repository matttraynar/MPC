#include "glwidget.h"

#include <iostream>

// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f,-1.0f, // triangle 2 : begin
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, // triangle 2 : end
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

GLWidget::GLWidget( QWidget* parent )
    : QGLWidget(parent ),
    m_vbo( QGLBuffer::VertexBuffer ),
    m_mouseDelta(0),
    m_cameraPos(4,3,3),
    m_mousePress(false),
    m_lastTime(0.0f)
{
    QSurfaceFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QSurfaceFormat::CoreProfile ); // Requires >=Qt-4.8.0

    glEnable(GL_CULL_FACE);
}

void GLWidget::initializeGL()
{
     QGLFormat glFormat = QGLWidget::format();


     glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );

     if ( !prepareShaderProgram( "shaders/vert.glsl", "shaders/frag.glsl" ) )
     {
         return;
     }

     m_vbo.create();

     m_vbo.setUsagePattern( QGLBuffer::StaticDraw );

     if ( !m_vbo.bind() )
     {
         qWarning() << "Could not bind vertex buffer to the context";
         return;
     }

     m_vbo.allocate( g_vertex_buffer_data, sizeof(g_vertex_buffer_data) );

     if ( !m_pgm.bind() )
     {
         qWarning() << "Could not bind shader program to context";
         return;
     }

     m_pgm.setAttributeBuffer( "vertex", GL_FLOAT, 0, 3 );
     m_pgm.enableAttributeArray( "vertex" );
}

bool GLWidget::prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath )
{
     // First we load and compile the vertex shader…
     bool result = m_pgm.addShaderFromSourceFile( QGLShader::Vertex, vertexShaderPath );
     if ( !result )
        qWarning() << m_pgm.log();

    // …now the fragment shader…
     result = m_pgm.addShaderFromSourceFile( QGLShader::Fragment, fragmentShaderPath );
     if ( !result )
         qWarning() << m_pgm.log();

    // …and finally we link them to resolve any references.
     result = m_pgm.link();
     if ( !result )
        qWarning() << "Could not link shader program:" << m_pgm.log();

    return result;
}


void GLWidget::resizeGL( int w, int h )
{
 // Set the viewport to window dimensions
 glViewport( 0, 0, w, qMax( h, 1 ) );
}

void GLWidget::paintGL()
{
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    update();

    QVector3D o(0,0,0);
    QVector3D camToOrig = o - m_cameraPos;

    camToOrig.normalize();

    QVector3D newCamPos = m_cameraPos + (m_mouseDelta * camToOrig);

    m_view.setToIdentity();

    m_view.lookAt(newCamPos + m_dir, QVector3D(0,0,0), QVector3D(0,1,0));

    m_proj.setToIdentity();
    m_proj.perspective(70.0f, float(width())/height(), 0.1f, 1000.0f);

    m_model.setToIdentity();

    m_mvp = m_proj * m_view * m_model;

    m_pgm.setUniformValue("MVP",m_mvp);

    // Draw stuff
    glDrawArrays( GL_TRIANGLES, 0, 12*3 );


}

void GLWidget::keyPressEvent( QKeyEvent* e )
{
     switch (e->key())
     {
     case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;

     default:
            QGLWidget::keyPressEvent( e );
     }
}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    int numDegrees = e->delta() / 8;
    int numSteps = numDegrees / 15;

    m_mouseDelta += numSteps;
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    m_xPos = e->globalX();
    m_yPos = e->globalY();

    QTime time;
    double cTime = QTime::currentTime().secsTo(time);
    float deltaTime = float(cTime - m_lastTime);

    m_lastTime = cTime;

    m_hAngle += 0.005f * deltaTime * float(width()/2 - m_xPos);
    m_vAngle += 0.005f * deltaTime * float(height()/2 - m_yPos);

    m_dir.setX(cos(m_vAngle) * sin(m_hAngle));
    m_dir.setY(sin(m_vAngle));
    m_dir.setZ(cos(m_vAngle) * cos(m_hAngle));


    m_right = QVector3D(sin(m_hAngle - 3.14f/2.0f),
                        0,
                        cos(m_hAngle - 3.14f/2.0f));

    m_up.crossProduct(m_right, m_dir);
}

