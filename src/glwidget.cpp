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
//    m_bullet->setGravity(0.0f, 0.0f, 0.0f);
    m_bullet->setGravity(0.0f, -9.8f, 0.0f);
    m_bullet->addGround();

    //Initialise variables for mouse control to 0
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
    m_zDis = 0;
    m_mouseDelta = 0;
    m_cameraPos = QVector3D(50.0f,50.0f,50.0f);

    m_isSimulating = false;
    m_drawMesh = false;
    m_drawSpheres = true;

    m_plastic = false;

    m_moveUp = false;
    m_moveDown = false;
    m_adjust = false;

    //Initialise the postition which will be used later
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
     glEnable(GL_PROGRAM_POINT_SIZE);

     //Create the correct shader program and if it fails immediately return
     if ( !prepareShaderProgram( "shaders/vert.glsl", "shaders/frag.glsl" ) )
     {
         exit(1);
     }

     //Bind the shader program to the current context
     m_pgm.bind();

     //Set an initial view matrix
     m_view.setToIdentity();
     m_view.lookAt(m_cameraPos,    // Camera Position
                   QVector3D(0.0f, 0.0f, 0.0f),    // Point camera looks towards
                   QVector3D(0.0f, 1.0f, 0.0f));   // Up vector

     //Start the QTimer with a step of 10
     startTimer(10);

     //Create an instance of the BtShape class
     BtShape* shapes = BtShape::instance();

     //Add two meshes to it
     shapes->addMesh("groundPlane","objFiles/ground.obj",QVector3D(1.0,1.0,1.0));
     shapes->addMesh("mesh","objFiles/cubeLARGE.obj",QVector3D(0.0,0.0,1.0));
     shapes->addSphere("sphere",1.0f);

     //Create a ground plane in the scene objects so that
     //it gets drawn
     createGround();

     createMesh("objFiles/cubeSTEP1.obj", "bunny", QVector3D(0,25,0));
     createMesh("objFiles/cubeSTEP2.obj", "cube", QVector3D(0,5,0));

     //Release the shader program
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

    m_pgm.setUniformValue("camPos", newCamPos);

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
    std::shared_ptr<Mesh> ground(new Mesh(QVector4D(1.0,1.0,1.0,1.0), "ground"));

    //Load the ground plane obj
    ground->loadMesh("objFiles/ground.obj");

    //Load the neccesary vaos and vbos
    ground->prepareMesh(m_pgm);

    //Add the pointer to the vector of scene objects
    m_sceneObjects.push_back(ground);
    m_sceneObjectPositions.push_back(QVector3D(0,0,0));

    //Release the shader program
    m_pgm.release();
}

void GLWidget::createTeapot()
{
    //Bind the shader program to the context
    m_pgm.bind();

    //Create a new mesh shared pointer and use the colour constructor
    std::shared_ptr<Mesh> teapot(new Mesh(QVector4D(0.9f,1.0f,1.0f,1.0f), "teapot"));

    //Load the teapot obj
    teapot->loadMesh("objFiles/cubeVoronoi.obj");

    //Load the neccesary vaos and vbos
    teapot->prepareMesh(m_pgm);

    //Generate the distance points used to speed up sphere packing
    teapot->generateDistanceField();
    teapot->preparePoints(m_pgm);

    //Pack the mesh with spheres
    teapot->packSpheres();

    //Add the pointer to the vector of scene objects
    m_sceneObjects.push_back(teapot);
    m_sceneObjects[1]->setWireMode();

    //Iterate over all the spheres in the pack
    for(int i = 0; i < m_sceneObjects[1]->getSphereNum(); ++i)
    {
        //For each sphere add a new btSphere to the bullet world (and at the
        //correct position
        m_bullet->addSphere(m_sceneObjects[1]->getSphereAt(i) + QVector3D(0,5,0), 1.0f, QVector3D(0,0,0));
    }


    //Get the number of collision objects in the world
    uint nBodies = m_bullet->getNumCollisionObjects();
    uint nSpheres = m_sceneObjects[1]->getSphereNum();

    int constCount = 0;

    //Create a new container for pairs
    std::vector< std::pair<uint, uint> > sphereIndices;

    qInfo()<<"Spheres: "<<nSpheres<<" Bodies: "<<nBodies;

    //Iterate through them
    for(uint i = 1; i < nBodies; ++i)
    {
        //Account for the ground plane (the first object in the bullet world)
        int sphereIndex = i - 1;

        //Get the possible connections the current sphere could have in
        //the sphere pack
        std::vector<QVector3D> spheresToConnect;
        teapot->getCloseSpheres(sphereIndex, spheresToConnect, sphereIndices);

        //Check if there are any candidate spheres
        if(spheresToConnect.size() == 0)
        {
            //We can ignore this sphere if there aren't any
            continue;
        }
        else
        {
            //There are some spheres that can be constrained to
            for(uint j = 0; j < sphereIndices.size(); ++j)
            {
                //Iterate through each one, check if the first sphere index
                //in the container of pairs is the same as the current
                //bullet rigid body we are testing
                uint sphereA = sphereIndices[j].first;

                if(sphereIndex == sphereA)
                {
                    //If it is find out what the body is paired to
                    uint sphereB = sphereIndices[j].second;

                    //The indices stored are created from the list of spheres,
                    //because we also have the static ground plane in the
                    //bullet world the indices need to be incremented
                    sphereA++;
                    sphereB++;

                    //Get the rigid bodies relating to the pair
                    btRigidBody* bodyA = m_bullet->getBodyAt(sphereA);
                    btRigidBody* bodyB = m_bullet->getBodyAt(sphereB);

                    //First get the transform of the first body
                    btTransform transA;
                    transA.setIdentity();
                    bodyA->getMotionState()->getWorldTransform(transA);

                    //Now get the second
                    btTransform transB;
                    transB.setIdentity();

                    //I originally tried doing this the same was as with A
                    //but found that this caused the two bodies to fly toward
                    //each other. Instead I convert the frame of B so that it
                    //is relative to A.
                    transB = (bodyA->getCenterOfMassTransform() * transA) * (bodyB->getCenterOfMassTransform().inverse());

                    //Add the constraint to the bullet world
                    m_bullet->addFixedConstraint(bodyA, bodyB, transA, transB);

                    //Mark the addition of a new constraint
                    constCount++;
                }
            }
        }
    }

    //Release the shader program
    m_pgm.release();
}

void GLWidget::createMesh(const char *filepath, const std::string name, QVector3D position)
{
    //Bind the shader program to the context
    m_pgm.bind();

    int index = 0;
    int newMeshPosition = 1000000000;

    if(checkExisting(name, index))
    {
        qInfo()<<"Mesh has already been added, duplicating";
        m_sceneObjects.push_back(m_sceneObjects[index]);
        newMeshPosition = (int)m_sceneObjects.size() - 1;
    }
    else
    {
        //Create a new mesh shared pointer and use the colour constructor
        std::shared_ptr<Mesh> mesh(new Mesh(QVector4D(0.9f,1.0f,1.0f,1.0f), name));

        //Load the teapot obj
        mesh->loadMesh(filepath);

        //Load the neccesary vaos and vbos
        mesh->prepareMesh(m_pgm);

        //Generate the distance points used to speed up sphere packing
        mesh->generateDistanceField();
        mesh->preparePoints(m_pgm);

        //Pack the mesh with spheres
        mesh->packSpheres();

        //Add the pointer to the vector of scene objects
        m_sceneObjects.push_back(mesh);

        newMeshPosition = (int)m_sceneObjects.size() - 1;
    }

    m_sceneObjectPositions.push_back(position);

    uint currentNBodies = m_bullet->getNumCollisionObjects();

    //Iterate over all the spheres in the pack
    for(int i = 0; i < m_sceneObjects[newMeshPosition]->getSphereNum(); ++i)
    {
        //For each sphere add a new btSphere to the bullet world (and at the
        //correct position
        m_bullet->addSphere(m_sceneObjects[newMeshPosition]->getSphereAt(i) + position, 1.0f, QVector3D(0,0,0));
    }


    //Get the number of collision objects in the world
    uint nBodies = m_bullet->getNumCollisionObjects();
    uint nSpheres = m_sceneObjects[newMeshPosition]->getSphereNum();

    int constCount = 0;

    //Create a new container for pairs
    std::vector< std::pair<uint, uint> > sphereIndices;

    qInfo()<<"Spheres: "<<nSpheres<<" Bodies: "<<nBodies;

    //Iterate through them
    for(uint i = currentNBodies; i < nBodies; ++i)
    {
        //Account for the ground plane (the first object in the bullet world)
        int sphereIndex = i - currentNBodies;

        //Get the possible connections the current sphere could have in
        //the sphere pack
        std::vector<QVector3D> spheresToConnect;
        m_sceneObjects[newMeshPosition]->getCloseSpheres(sphereIndex, spheresToConnect, sphereIndices);

        //Check if there are any candidate spheres
        if(spheresToConnect.size() == 0)
        {
            //We can ignore this sphere if there aren't any
            continue;
        }
        else
        {
            //There are some spheres that can be constrained to
            for(uint j = 0; j < sphereIndices.size(); ++j)
            {
                //Iterate through each one, check if the first sphere index
                //in the container of pairs is the same as the current
                //bullet rigid body we are testing
                uint sphereA = sphereIndices[j].first;

                if(sphereIndex == sphereA)
                {
                    //If it is find out what the body is paired to
                    uint sphereB = sphereIndices[j].second;

                    //The indices stored are created from the list of spheres,
                    //because we also have the static ground plane in the
                    //bullet world the indices need to be incremented
                    sphereA += currentNBodies;
                    sphereB += currentNBodies;

                    //Get the rigid bodies relating to the pair
                    btRigidBody* bodyA = m_bullet->getBodyAt(sphereA);
                    btRigidBody* bodyB = m_bullet->getBodyAt(sphereB);

                    //First get the transform of the first body
                    btTransform transA;
                    transA.setIdentity();
                    bodyA->getMotionState()->getWorldTransform(transA);

                    //Now get the second
                    btTransform transB;
                    transB.setIdentity();

                    //I originally tried doing this the same was as with A
                    //but found that this caused the two bodies to fly toward
                    //each other. Instead I convert the frame of B so that it
                    //is relative to A.
                    transB = (bodyA->getCenterOfMassTransform() * transA) * (bodyB->getCenterOfMassTransform().inverse());

                    //Add the constraint to the bullet world
                    m_bullet->addFixedConstraint(bodyA, bodyB, transA, transB);

                    //Mark the addition of a new constraint
                    constCount++;
                }
            }
        }
    }

    //Release the shader program
    m_pgm.release();
}

bool GLWidget::checkExisting(const std::string name, int &position)
{
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->getName() == name)
        {
            position = i;
            return true;
        }
    }

    return false;
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

    //Create a sphere we are going to draw in the correct places
    Mesh sphere;
    sphere.loadMesh("objFiles/sphere.obj");
    sphere.prepareMesh(m_pgm);
    sphere.setColour(QVector4D(1.0,0.0,0.0,1.0));

    //Get the number of bullet bodies
    uint nBodies = m_bullet->getNumCollisionObjects();

    QVector3D middlePosition(0,0,0);
    int numSpheres = 0;

    //Iterate through them
    for(uint i = 0; i < nBodies; ++i)
    {
        //Get the position of the bullet mesh
        m_position = m_bullet->getTransform(i);

        //Load the viewing and object transformation matrices
        //to the shader program
        loadShaderMatrices();

        //Get the name of the bullet object
        std::string name = m_bullet->getBodyNameAt(i);

        //Check for the teapot
        if(name == "mesh")
        {
            //Set the colour of the object in the shader
            m_pgm.setUniformValue("mCol",m_sceneObjects[i]->getColour());

            //Draw the object
            m_sceneObjects[i]->draw();
        }
        //Check for a sphere
        else if(name == "sphere")
        {
            middlePosition += m_position;
            numSpheres++;

            //Set the colour of the object in the shader
            sphere.setColour(QVector4D(1 - (1.0f/i), 0.0f, 0.0f, 0.0f));
            m_pgm.setUniformValue("mCol",sphere.getColour());

            //Draw an instance of the sphere
            if(m_drawSpheres)
            {
                sphere.draw();
            }
        }
    }


    //Finally create a new position for the ground plane and load it
    m_position = QVector3D(0.0f, 0.0f, 0.0f);
    loadShaderMatrices();

    //Again set the shader colour for the plane
    m_pgm.setUniformValue("mCol",m_sceneObjects[1]->getColour());

    //The plane is always created first, so we can assume it's index
    //which we can use to draw
    m_sceneObjects[0]->draw();

    //Check if the mesh is being drawn
    if(m_drawMesh)
    {
        //NEEDS EDITTING
        for(uint i = 1; i < m_sceneObjects.size(); ++i)
        {
            m_sceneObjects[i]->draw();
        }
    }


    //Finally release the shader program
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
    int objCount = 1;

    switch(e->key())
    {
    case Qt::Key_W:
        //User pressed 'W' so update the wireframe mode
        //of each mesh in the scene
        for(uint i = 1; i < m_sceneObjects.size(); ++i)
        {
            m_sceneObjects[i]->setWireMode();
        }

        break;

    case Qt::Key_S:
        m_drawSpheres = !m_drawSpheres;
        break;

    case Qt::Key_Escape:
        //User pressed 'esc' so close the window
        close();
        break;

    case Qt::Key_Space:
        //User pressed 'space' so toggle the
        //simulation
        m_isSimulating = !m_isSimulating;
        break;

    case Qt::Key_M:
        //User pressed 'm' so toggle the
        //drawing of the mesh
        m_drawMesh = !m_drawMesh;
        break;

    case Qt::Key_P:
        m_plastic = !m_plastic;
        break;

    case Qt::Key_Up:
        if(m_moveDown)
        {
            m_moveDown = false;
        }
        else
        {
            m_moveUp = true;
            m_adjustPos[1] += 50.0f;
        }
        break;

    case Qt::Key_Down:
        if(m_moveUp)
        {
            m_moveUp = false;
        }
        else
        {
            m_moveDown = true;
            m_adjustPos[1] -= 50.0f;
        }
        break;

    case Qt::Key_Home:
        //User pressed 'home' so stop the simulation
        m_bullet->stop();

        //Reset all the sphere positions
        for(int i = 0; i < m_sceneObjects.size(); ++i)
        {
            for(int j = 0; j < m_sceneObjects[i]->getSphereNum(); ++j)
            {
                m_bullet->reset(m_sceneObjects[i]->getSphereAt(j) + m_sceneObjectPositions[i], objCount);
                objCount++;
            }
        }

        //Step the simulation once so that everything
        //changes properly
        m_bullet->step(1.0f/60.0f, 10);

        //Stop the simulation
        m_isSimulating = false;

        break;

    default:
        break;
    }

    //Redraw with the changes in mind
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
    //Getting a warning because the timer event isn't being used
    e;

    //Check if the program should be simulating
    if(m_isSimulating)
    {
        if(m_moveUp)
        {
            m_bullet->moveBodies(m_adjustPos);
            m_adjust = true;
            m_moveUp = false;
        }
        else if(m_moveDown)
        {
            m_bullet->moveBodies(m_adjustPos);
            m_adjust = true;
            m_moveDown = false;

        }
        else if(m_adjust == true)
        {
            m_bullet->stopAdjusting();
            m_adjustPos = QVector3D(0,0,0);
            m_adjust = false;
        }

//        if(m_plastic)
//        {
//            m_bullet->checkPlastic();
//        }

        //Check nothing is moving too fast
//        m_bullet->checkVelocities();

        //Step the simulation
        m_bullet->step(1.0f/60.0f, 10);

        //Update the window
        update();
    }
}
