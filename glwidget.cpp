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
    m_vbo( QOpenGLBuffer::VertexBuffer )

{
    QSurfaceFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QSurfaceFormat::CoreProfile ); // Requires >=Qt-4.8.0

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    m_mouseDelta = 0;
    m_cameraPos = QVector3D(4,3,3);
}

GLWidget::~GLWidget()
{

}

void GLWidget::initializeGL()
{
     glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );

     if ( !prepareShaderProgram( "shaders/vert.glsl", "shaders/frag.glsl" ) )
     {
         return;
     }

     createGeometry();

     m_view.setToIdentity();
     m_view.lookAt(QVector3D(0.0f, 0.0f, 1.2f),    // Camera Position
                   QVector3D(0.0f, 0.0f, 0.0f),    // Point camera looks towards
                   QVector3D(0.0f, 1.0f, 0.0f));   // Up vector


     /*m_vbo.create();

     m_vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );

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
     m_pgm.enableAttributeArray( "vertex" );*/
}

void GLWidget::createGeometry()
{
    if(!m_loader.loadFromFile("objFiles/cube.obj"))
    {
        qWarning() << "ModelLoader didn't load the file properly" << m_pgm.log();
        exit(1);
    }

    QVector<float>* v;
    QVector<float>* n;
    QVector<uint>*  i;

    m_loader.getBufferData(&v, &n, &i);

    if(!m_vao.isCreated())
    {
        m_vao.create();
    }

    m_vao.bind();

    m_vbo.create();
    m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo.bind();

    m_vbo.allocate(&(*v)[0], v->size() * sizeof((*v)[0]));

    m_pgm.enableAttributeArray(0);
    m_pgm.setAttributeBuffer(0, GL_FLOAT, 0, 3);

    m_nbo.create();
    m_nbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_nbo.bind();

    m_nbo.allocate(&(*n)[0], n->size() * sizeof((*n)[0]));

    m_pgm.enableAttributeArray(1);
    m_pgm.setAttributeBuffer(1, GL_FLOAT, 0, 3);

    m_ibo.create();
    m_ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_ibo.bind();

    m_ibo.allocate(&(*i)[0], i->size() * sizeof((*i)[0]));

    m_vao.release();
}

bool GLWidget::prepareShaderProgram( const QString& vertexShaderPath, const QString& fragmentShaderPath )
{
     // First we load and compile the vertex shader…
     bool result = m_pgm.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath );
     if ( !result )
        qWarning() << m_pgm.log();

    // …now the fragment shader…
     result = m_pgm.addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath );
     if ( !result )
         qWarning() << m_pgm.log();

    // …and finally we link them to resolve any references.
     result = m_pgm.link();
     if ( !result )
        qWarning() << "Could not link shader program:" << m_pgm.log();

    return result;
}

void GLWidget::drawNode(const QMatrix4x4 &model, const Node *node, QMatrix4x4 parent)
{
    QMatrix4x4 local = parent * node->trans;
    QMatrix4x4 mv = m_view * model * local;

    m_pgm.setUniformValue("MV",mv);
    m_pgm.setUniformValue("N",mv.normalMatrix());
    m_pgm.setUniformValue("MVP",m_proj*mv);

    for(int i = 0; i < node->meshes.size(); ++i)
    {
        const Mesh& m = *node->meshes[i];

        glDrawElements(GL_TRIANGLES,
                       m.iCount,
                       GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(m.iOffset * sizeof(uint)));
    }

    /*std::cout<<"Size :"<<node->nodes.size()<<'\n';
    for(int i = 0; i < node->nodes.size(); ++i)
    {
        drawNode(model, &node->nodes[i], local);
    }*/
}

void GLWidget::draw()
{
    QMatrix4x4 model;
    model.translate(-0.2f, 0.0f, 0.5f);
    model.rotate(55.0f, 0.0f, 1.0f, 0.0f);

    drawNode(model, m_loader.getNodeData(), QMatrix4x4());
}

void GLWidget::resizeGL( int w, int h )
{
    // Set the viewport to window dimensions
    glViewport( 0, 0, w, h );

    m_proj.setToIdentity();
    m_proj.perspective(70.0f, float(w)/h, 0.1f, 1000.0f);
    update();


}

void GLWidget::paintGL()
{
    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    update();

    m_pgm.bind();
    m_vao.bind();

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
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    //draw();

    m_vao.release();


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


    m_hAngle += 0.005f * float(width()/2 - m_xPos);
    m_vAngle += 0.005f * float(height()/2 - m_yPos);

    m_dir.setX(cos(m_vAngle) * sin(m_hAngle));
    m_dir.setY(sin(m_vAngle));
    m_dir.setZ(cos(m_vAngle) * cos(m_hAngle));


    m_right = QVector3D(sin(m_hAngle - 3.14f/2.0f),
                        0,
                        cos(m_hAngle - 3.14f/2.0f));

    m_up.crossProduct(m_right, m_dir);
}

