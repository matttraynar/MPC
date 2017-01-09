#include "glwidget.h"

#include <iostream>

GLWidget::GLWidget( QWidget* parent )
    : QGLWidget(parent )
{
    //Setup the format for the window
    QSurfaceFormat glFormat;
    glFormat.setVersion( 3, 3 );
    glFormat.setProfile( QSurfaceFormat::CoreProfile ); // Requires >=Qt-4.8.0

    //Create a new bullet physics world and add the ground plane
    m_bullet.reset(new BtWorld());
    m_bullet->setGravity(0.0f, -9.8f, 0.0f);
    m_bullet->addGround();

    m_simulationStep = 1.0f/60.0f;

    //The radius of the spheres being packed
    m_radius = 1.0f;

    //Initialise variables for mouse control to 0
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
    m_zDis = 0;
    m_mouseDelta = 0;
    m_cameraPos = QVector3D(50.0f,50.0f,50.0f);

    //Set the booleans which dictate drawing
    m_isSimulating = false;
    m_drawMesh = false;
    m_drawSpheres = true;

    //These variables are activated when the user
    //moves the mesh, set them all to false for now
    m_moveUp = false;
    m_moveDown = false;
    m_adjust = false;

    //Initialise the postition which will be used later
    m_position = QVector3D();
}

GLWidget::~GLWidget()
{
    //Clean up the BtShape incase an instance was made
    BtShape::destroy();
}

void GLWidget::setWorldColour(QColor colour)
{
    glClearColor((float)colour.red()/255.0f,
                      (float)colour.green()/255.0f,
                      (float)colour.blue()/255.0f,
                      1.0f );
}

void GLWidget::setPlaneColour(QColor colour)
{
    m_sceneObjects[0]->setColour(QVector4D((float)colour.red()/255.0f,
                                                 (float)colour.green()/255.0f,
                                                 (float)colour.blue()/255.0f,
                                                 1.0f ));
}

void GLWidget::addNewMesh(QString fileName, QString meshName, QVector3D position)
{
    createMesh(fileName.toStdString().c_str(), meshName.toStdString(), position);

    update();
}

void GLWidget::setMeshShader(QString meshName, ShaderSettings &settings)
{
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->getName() == meshName.toStdString())
        {
            qInfo()<<"updating shader";
            m_sceneObjects[i]->setColour(QVector4D(settings.red,
                                                                          settings.green,
                                                                          settings.blue,
                                                                          1.0f ));

            m_sceneObjects[i]->setWireMode(settings.wireframe);

            //ADD SKIN
            m_drawMeshStates[i] = settings.drawMesh;
            update();
            break;
        }
    }
}

void GLWidget::runDistanceField(QString meshName, DistanceFieldSettings &settings)
{
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->getName() == meshName.toStdString())
        {
            m_sceneObjects[i]->generateDistanceField(settings);
            update();
            break;
        }
    }
}

void GLWidget::runSpherePack(QString meshName, SpherePackSettings &settings)
{
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->getName() == meshName.toStdString())
        {
            m_sceneObjects[i]->runSpherePackAlgorithm(settings);

            qInfo()<<"Finished sphere pack";

            //Get the number of bodies in the bullet world *before* adding
            //the sphere from the sphere pack
            uint currentNBodies = m_bullet->getNumCollisionObjects();

            //A container for storing the sphere positions (used later)
            vector_V spherePositions = m_sceneObjects[i]->m_spherePack->getSpheres();

            uint sphereNum = m_sceneObjects[i]->m_spherePack->getSphereNum();

            //Iterate over all the spheres in the pack
            for(uint j = 0; j < sphereNum; ++j)
            {
                //For each sphere add a new btSphere to the bullet world (and add the displacement
                //specified by 'position')
                spherePositions.push_back(m_sceneObjects[i]->m_spherePack->getSphereAt(j) + m_sceneObjectPositions[i]);
                m_bullet->addSphere(m_sceneObjects[i]->m_spherePack->getSphereAt(j) + m_sceneObjectPositions[i], 1.0f, QVector3D(0,0,0));
            }

            //Get the number of collision objects in the world now
            uint nBodies = m_bullet->getNumCollisionObjects();

            //Mark how many spheres were added for this mesh
            m_sphereNumbers.push_back(sphereNum);

            update();
            break;
        }
    }
}

void GLWidget::toggleSimulation(bool isSimulating)
{
    m_isSimulating = isSimulating;
}

void GLWidget::resetSimulation()
{
    //User pressed 'home' so stop the simulation
    m_bullet->stop();

    int objCount = 0;

    //Reset all the sphere positions
    for(int i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->hasSpherePack())
        {
            for(uint j = 0; j < m_sceneObjects[i]->m_spherePack->getSphereNum(); ++j)
            {
                m_bullet->reset(m_sceneObjects[i]->m_spherePack->getSphereAt(j) + m_sceneObjectPositions[i], objCount);
                objCount++;
            }
        }
        else
        {
            objCount++;
        }
    }

    //Step the simulation once so that everything
    //changes properly
    m_bullet->step(1.0f/60.0f, 10);

    //Stop the simulation
    m_isSimulating = false;
}

void GLWidget::setSimulation(float stepValue)
{
    m_simulationStep = stepValue;
}

void GLWidget::initializeGL()
{
    //Set the background colour for the window
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

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

     //Add add the sphere shape to the bullet world for later
     shapes->addSphere("sphere", m_radius);

     //Create a ground plane in the scene objects so that
     //it gets drawn
     createGround();

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

void GLWidget::loadShaderMatrices(float radius)
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

    //Calculate the MVP
    m_mvp = m_proj * m_view * m_model;

    //Pass the MVP into the shader
    m_pgm.setUniformValue("M",m_model);
    m_pgm.setUniformValue("MVP",m_mvp);
    m_pgm.setUniformValue("camPos", newCamPos);//* m_model);
    m_pgm.setUniformValue("scale", radius);
}

void GLWidget::createGround()
{
     //Bind the shader program to the context
    m_pgm.bind();

    //Create a new mesh shared pointer and use the colour constructor
    std::shared_ptr<Mesh> ground(new Mesh(QVector4D(0.9f,1.0f,1.0f,1.0f), "ground"));

    //Load the ground plane obj
    ground->loadMesh("objFiles/ground.obj");

    //Load the neccesary vaos and vbos
    ground->prepareMesh(m_pgm);

    //Add the pointer to the vector of scene objects
    m_sceneObjects.push_back(ground);
    m_sceneObjectPositions.push_back(QVector3D(0,0,0));
    m_drawMeshStates.push_back(true);

    //Release the shader program
    m_pgm.release();
}

void GLWidget::createMesh(const char *filepath, const std::string name, QVector3D position)
{
    //Bind the shader program to the context
    m_pgm.bind();

    //Create two variables which will be overwritten.
    //newMeshPosition is set to a value that should
    //throw an error if it doesn't get overwritten
    int index = 0;
    int newMeshPosition = 1000000000;

    //Check if the mesh has actually already been added
    if(checkExisting(name, index))
    {
        qInfo()<<"Mesh has already been added, duplicating";
        std::shared_ptr<Mesh> mesh(new Mesh(*m_sceneObjects[index]));

        //These two methods make sure the mesh VAOs which do not
        //have copy constructors/assignment operators (an so couldn't
        //be copied by the Mesh copy constructor) are created
        mesh->prepareMesh(m_pgm);
        mesh->prepareSkinnedMesh(m_pgm);

        //Add the mesh to the list of objects
        m_sceneObjects.push_back(mesh);

        //Mark its new position
        newMeshPosition = (int)m_sceneObjects.size() - 1;
    }
    else
    {
        //Create a new mesh shared pointer and use the colour constructor
        std::shared_ptr<Mesh> mesh(new Mesh(QVector4D(1.0f,0.1f,0.1f,1.0f), name));

        //Load the teapot obj
        mesh->loadMesh(filepath);

        //Load the neccesary vaos and vbos
        mesh->prepareMesh(m_pgm);

        //Sphere pack the mesh
//        mesh->runSpherePackAlgorithm(m_radius);

        //Set up the skinned vertex positions
//        mesh->skinMeshToSpheres(2);

        //Add the pointer to the vector of scene objects
        m_sceneObjects.push_back(mesh);

        //And store the position
        newMeshPosition = (int)m_sceneObjects.size() - 1;
    }

    //Finally add the desired position of the mesh to a container
    m_sceneObjectPositions.push_back(position);
    m_drawMeshStates.push_back(true);

    //Get the number of bodies in the bullet world *before* adding
    //the sphere from the sphere pack
//    uint currentNBodies = m_bullet->getNumCollisionObjects();

//    //A container for storing the sphere positions (used later)
//    vector_V spherePositions;

//    //Iterate over all the spheres in the pack
//    for(uint i = 0; i < m_sceneObjects[newMeshPosition]->m_spherePack->getSphereNum(); ++i)
//    {
//        //For each sphere add a new btSphere to the bullet world (and add the displacement
//        //specified by 'position')
//        spherePositions.push_back(m_sceneObjects[newMeshPosition]->m_spherePack->getSphereAt(i) + position);
//        m_bullet->addSphere(m_sceneObjects[newMeshPosition]->m_spherePack->getSphereAt(i) + position, 1.0f, QVector3D(0,0,0));
//    }

//    //Get the number of collision objects in the world now
//    uint nBodies = m_bullet->getNumCollisionObjects();
//    uint nSpheres = m_sceneObjects[newMeshPosition]->m_spherePack->getSphereNum();

//    //Mark how many spheres were added for this mesh
//    m_sphereNumbers.push_back(nSpheres);

//    int constCount = 0;

//    qInfo()<<"Spheres: "<<nSpheres<<" Bodies: "<<nBodies;

//    //A container for storing the constraints of the spheres by index
//    std::vector< std::pair<uint, uint> > sphereConstraints;

//    //Iterate through the bodies in the 'world'
//    for(uint i = currentNBodies; i < nBodies; ++i)
//    {
//        //Create a new container for pairs of sphere indices
//        std::vector< std::pair<uint, uint> > sphereIndices;

//        //Mark the index of the sphere (accounting for any other objects
//        //in the bullet world).
//        uint sphereIndex = i - currentNBodies;

//        //Get the possible connections the current sphere could have in
//        //the sphere pack
//        std::vector<QVector3D> spheresToConnect;
//        m_sceneObjects[newMeshPosition]->m_spherePack->getCloseSpheres(sphereIndex, spheresToConnect, sphereIndices, 300);

//        //Check if there are any candidate spheres
//        if(spheresToConnect.size() == 0)
//        {
//            //We can ignore this sphere if there aren't any
//            continue;
//        }
//        else
//        {
//            //There are some spheres that can be constrained to
//            for(uint j = 0; j < sphereIndices.size(); ++j)
//            {
//                //Iterate through each one, check if the first sphere index
//                //in the container of pairs is the same as the current
//                //bullet rigid body we are testing
//                uint sphereA = sphereIndices[j].first;

//                if(sphereIndex == sphereA)
//                {
//                    //If it is find out what the body is paired to
//                    uint sphereB = sphereIndices[j].second;

//                    sphereConstraints.push_back(std::make_pair(sphereA, sphereB));

//                    //The indices stored are created from the list of spheres,
//                    //because we may also have other objects in the
//                    //bullet world the value needs to be adjusted
//                    sphereA += currentNBodies;
//                    sphereB += currentNBodies;

//                    //Get the rigid bodies relating to the pair
//                    btRigidBody* bodyA = m_bullet->getBodyAt(sphereA);
//                    btRigidBody* bodyB = m_bullet->getBodyAt(sphereB);

//                    //First get the transform of the first body
//                    btTransform transA;
//                    transA.setIdentity();
//                    bodyA->getMotionState()->getWorldTransform(transA);

//                    //Now get the second
//                    btTransform transB;
//                    transB.setIdentity();

//                    //I originally tried doing this the same way as with A
//                    //but found that this caused the two bodies to fly toward
//                    //each other. Instead I convert the frame of B so that it
//                    //is relative to A.
//                    transB = (bodyA->getCenterOfMassTransform() * transA) * (bodyB->getCenterOfMassTransform().inverse());

//                    //Add the constraint to the bullet world
//                    m_bullet->addFixedConstraint(bodyA, bodyB, transA, transB);

//                    //Mark the addition of a new constraint
//                    constCount++;
//                }
//            }
//        }
//    }

//    //Add the constraints to a container so they can be draw later
//    m_constraints.push_back(sphereConstraints);

    //Release the shader program
    m_pgm.release();
}

bool GLWidget::checkExisting(const std::string name, int &position)
{
    //Iterate through all the objects in the scene and
    //check their name
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        //If the name already exits mark the index
        //it appears at and exit the function as we
        //don't need to continue checking for it
        if(m_sceneObjects[i]->getName() == name)
        {
            position = i;
            return true;
        }
    }

    return false;
}

void GLWidget::updateConstraintDrawing()
{
    //Clear the constraint vertices
    constraintVerts.clear();

    //Iterate through the container of constraints
    for(uint i = 0; i < m_constraints.size(); ++i)
    {
        //The container is 2x2 so iterate through this
        //next container
        for(uint j = 0; j < m_constraints[i].size(); ++j)
        {
            //Add the position of the two constrained spheres
            //to the constrained verts (so later we can draw a
            //line between them)
            constraintVerts.push_back(m_spherePositions[i][m_constraints[i][j].first]);
            constraintVerts.push_back(m_spherePositions[i][m_constraints[i][j].second]);
        }
    }

    //Check if the constraint VAO already exists, create
    //it if not
    if(!m_vaoConstraint.isCreated())
    {
        m_vaoConstraint.create();
    }

    //Bind the vao to the current context
    m_vaoConstraint.bind();

    //Do the same check/bind for the constraint VBO
    if(!m_vboConstraint.isCreated())
    {
        m_vboConstraint.create();
    }

    m_vboConstraint.bind();

    //Store the constraint positions in the VBO
    m_vboConstraint.allocate(&constraintVerts[0], (int)constraintVerts.size() * sizeof(GLfloat) * 3);

    //Tell the shader program where this data wil go
    m_pgm.enableAttributeArray("vertexPos");
    m_pgm.setAttributeArray("vertexPos", GL_FLOAT, 0, 3);

    //We can now release the vbo and the vao
    m_vboConstraint.release();

    m_vaoConstraint.release();
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

    if(m_sceneObjects.size() > 1)
    {
        //Create a sphere which will be used to draw the sphere pack
        Mesh sphere(QVector4D(1.0f, 0.0f, 0.0f, 1.0f), "sphere");
        sphere.loadMesh("objFiles/sphere.obj");
        sphere.prepareMesh(m_pgm);

        //Get the number of bullet bodies
        uint nBodies = m_bullet->getNumCollisionObjects();

        //A container used to store the positions of the spheres
        //in the sphere pack
        vector_V spherePositions;

        //Two counters, one for the number of bodies/meshes in
        //the scene and the other for the number of spheres
        int numSpheres = 0;
        int bodyNumber = 0;

        m_spherePositions.clear();

        //Iterate through them
        for(uint i = 0; i < nBodies; ++i)
        {
            //Get the position of the bullet mesh
            m_position = m_bullet->getTransform(i);

            //Get the name of the bullet object
            std::string name = m_bullet->getBodyNameAt(i);

            //Check for a sphere
            if(name == "sphere")
            {
                //Add the position and increment the count
                spherePositions.push_back(m_position);

                numSpheres++;

                //Check to see if all the spheres from the current
                //mesh have been added to the position container
                if(numSpheres == m_sphereNumbers[bodyNumber])
                {
                    //We have, add the positions to the container of all
                    //sphere positions and clear the vector for further use
                    m_spherePositions.push_back(spherePositions);

                    spherePositions.clear();

                    //Reset the sphere count
                    numSpheres = 0;

                    //Increment the body number
                    bodyNumber++;
                }
            }
        }

        //Iterate through all the objects in the mesh
        int sphereIndex = 0;
        for(uint i = 0; i < m_sceneObjects.size(); ++i)
        {
            //Check if the object has been sphere packed
            if(m_sceneObjects[i]->hasSpherePack())
            {
                if(m_sceneObjects[i]->isSkinned())
                {
                    //Update the skinned mesh
                    m_sceneObjects[i]->updateSkinnedMesh(m_spherePositions[sphereIndex]);

                    //Update the skinned mesh VAO for drawing later
                    m_sceneObjects[i]->prepareSkinnedMesh(m_pgm);
                }

                sphereIndex++;
            }
        }


        //Set the drawing colour to the sphere colour
        m_pgm.setUniformValue("mCol",sphere.getColour());

        //Are we currently drawing the spheres or not?
        if(m_drawSpheres)
        {
            //We are, iterate through the 2x2 sphere position
            //matrix
            for(uint i = 0; i < m_spherePositions.size(); ++i)
            {
                for(uint j = 0; j < m_spherePositions[i].size(); ++j)
                {
                    //Set the drawing position accordingly and load
                    //it to the shader. Make sure to set the drawing
                    //scale to the radius (the sphere mesh has a
                    //radius of one to facilitate this).
                    m_position = m_spherePositions[i][j];

                    loadShaderMatrices(m_radius);

                    //Finally draw the sphere
                    sphere.draw();
                }
            }
        }
        //    else
        //    {
        //        updateConstraintDrawing();

        //        m_vaoConstraint.bind();
        //        m_pgm.setUniformValue("mCol",sphere.getColour());

        //        glLineWidth(0.25f);
        //        glDrawArrays(GL_LINES, 0, (int)constraintVerts.size());
        //        m_vaoConstraint.release();
        //    }

        //Reset the drawing position
        m_position = QVector3D(0,0,0);
        loadShaderMatrices(1.0f);

        //If it is, iterate through the objects in the scene starting
        //at 1 because 0 (the ground plane) has already been drawn
        for(uint i = 1; i < m_sceneObjects.size(); ++i)
        {
            if(m_drawMeshStates[i])
            {

                //Set the right colour
                m_pgm.setUniformValue("mCol",m_sceneObjects[i]->getColour());

                if(m_sceneObjects[i]->isSkinned())
                {
                    //Because this uses the skinned verts which were updated later
                    //the drawing position doesn't need updating
                    m_sceneObjects[i]->draw();
                }
                else
                {
                    m_position = m_sceneObjectPositions[i];

                    loadShaderMatrices(1.0f);

                    m_sceneObjects[i]->draw();
                }
            }
        }

    }
    //Reset the drawing position
    m_position = QVector3D(0,0,0);
    loadShaderMatrices(1.0f);

    //Again set the shader colour for the plane
    m_pgm.setUniformValue("mCol",m_sceneObjects[0]->getColour());

    //The plane is always created first, so we can assume it's index
    //which we can use to draw
    m_sceneObjects[0]->draw();

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
    int objCount = 0;

    switch(e->key())
    {
    case Qt::Key_W:
        //User pressed 'W' so update the wireframe mode
        //of each mesh in the scene
        for(uint i = 1; i < m_sceneObjects.size(); ++i)
        {
//            m_sceneObjects[i]->setWireMode();
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
            if(m_sceneObjects[i]->hasSpherePack())
            {
                for(uint j = 0; j < m_sceneObjects[i]->m_spherePack->getSphereNum(); ++j)
                {
                    m_bullet->reset(m_sceneObjects[i]->m_spherePack->getSphereAt(j) + m_sceneObjectPositions[i], objCount);
                    objCount++;
                }
            }
            else
            {
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

        //Step the simulation
        m_bullet->step(m_simulationStep, 10);

        //Update the window
        update();
    }
}
