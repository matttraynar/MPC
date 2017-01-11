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

    //Set the initial value of the simulation step
    m_simulationStep = 1.0f/60.0f;

    //Initialise variables for mouse control to 0
    m_xRot = 0;
    m_yRot = 0;
    m_zRot = 0;
    m_zDis = 0;
    m_mouseDelta = 0;
    m_cameraPos = QVector3D(25.0f,25.0f,25.0f);

    //Set the booleans which dictate drawing
    m_isSimulating = false;

    //These variables are activated when the user
    //moves the mesh, set them all to false for now
    m_moveUp = false;
    m_moveDown = false;

    //Initialise the postition which will be used later
    m_position = QVector3D(0,0,0);
}

GLWidget::~GLWidget()
{

}

//----------------------------------------------------------------------------- SLOTS -----------------------------------------------------------------------------//

void GLWidget::setWorldColour(QColor colour)
{
    //Slot used to set the background colour of the window
    glClearColor((float)colour.red()/255.0f,
                      (float)colour.green()/255.0f,
                      (float)colour.blue()/255.0f,
                      1.0f );
}

void GLWidget::setPlaneColour(QColor colour)
{
    //Slot which adjusts the colour of the plane. As the plane
    //is always added first to the scene it's index in the container
    //of objects can be assumed
    m_sceneObjects[0]->setColour(QVector4D((float)colour.red()/255.0f,
                                                 (float)colour.green()/255.0f,
                                                 (float)colour.blue()/255.0f,
                                                 1.0f ));
}

void GLWidget::addNewMesh(QString fileName, QString meshName, QVector3D position)
{
    //Adds a new mesh to the scene and then updates
    createMesh(fileName.toStdString().c_str(), meshName.toStdString(), position);
    update();
}

void GLWidget::removeMesh(QString meshName)
{
    //Remove a mesh from the scene
    uint numObjects = (uint)m_sceneObjects.size();

    int bodyCount = 0;

    for(uint i = 0; i < numObjects; ++i)
    {
        //First we check the name of each object
        if(m_sceneObjects[i]->getName() == meshName.toStdString())
        {
            //If the object has been sphere packed already we need to take care of that
            if(m_sceneObjects[i]->hasSpherePack())
            {
                //Delete all the neccesary data
                m_sphereNumbers.erase(m_sphereNumbers.begin() + i - 1);
                m_spherePositions.erase(m_spherePositions.begin() + i - 1);
                m_sphereRadii.erase(m_sphereRadii.begin() + i - 1);
                m_drawSphereStates.erase(m_drawSphereStates.begin() + i - 1);
                m_drawConstraintStates.erase(m_drawConstraintStates.begin() + i - 1);

                //Remove any constraints
                if(m_constraints.size() > 0)
                {
                    m_constraints.erase(m_constraints.begin() + i - 1);
                }

                //Remove the spheres from the bullet world
                for(uint j = 0; j < m_sceneObjects[i]->m_spherePack->getSphereNum(); ++j)
                {
                    m_bullet->remove(j + bodyCount - 1);
                }

                //And remove from the container of 'Body' types
                m_bullet->removeBodies(bodyCount - 1, m_sceneObjects[i]->m_spherePack->getSphereNum());
            }

            //Finally delete the position data, the draw mesh state and then the actual mesh
            //object from the scene
            m_sceneObjectPositions.erase(m_sceneObjectPositions.begin() + i);
            m_drawMeshStates.erase(m_drawMeshStates.begin() + i);
            m_sceneObjects.erase(m_sceneObjects.begin() + i);

            //As this will only be done once we can break here
            break;
        }

        //Update numbers just so they can be used when removing bodies from
        //the bullet world
        if(m_sceneObjects[i]->hasSpherePack())
        {
            bodyCount += m_sceneObjects[i]->m_spherePack->getSphereNum();
        }
        else
        {
            bodyCount++;
        }
    }

    //And update the scene
    update();
}

void GLWidget::setMeshShader(QString meshName, ShaderSettings &settings)
{
    //Iterate over the objects in the scene
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        //Check their names
        if(m_sceneObjects[i]->getName() == meshName.toStdString())
        {
            //We've found the mesh so update all the neccesary values using the
            //data passed in the settings
            m_sceneObjects[i]->setColour(QVector4D(settings.red,
                                                                          settings.green,
                                                                          settings.blue,
                                                                          1.0f ));

            m_sceneObjects[i]->setWireMode(settings.wireframe);

            m_meshSkinStates[i - 1] = settings.skinMesh;
            m_drawMeshStates[i] = settings.drawMesh;

            if(settings.skinMesh)
            {
                m_sceneObjects[i]->skinMeshToSpheres(4);
                m_sceneObjects[i]->updateSkinnedMesh(m_sceneObjects[i]->m_spherePack->getSpheres());
            }

            //Finally update the window and break (to avoid any unnessecary loops)
            update();
            break;
        }
    }
}

void GLWidget::runDistanceField(QString meshName, DistanceFieldSettings &settings)
{
    //Calculate the signed distance field for the given mesh using the settings provided
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
    //Similar to the distance field, only this function also adds the spheres to the bullet world
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->getName() == meshName.toStdString())
        {
            //First run the algorithm
            m_sceneObjects[i]->runSpherePackAlgorithm(settings);

            //Then store the radius so it can be drawn properly later
            m_sphereRadii.push_back(settings.radius);

            //A container for storing the sphere positions (used later)
            vector_V spherePositions = m_sceneObjects[i]->m_spherePack->getSpheres();

            uint sphereNum = m_sceneObjects[i]->m_spherePack->getSphereNum();

            //Iterate over all the spheres in the pack
            for(uint j = 0; j < sphereNum; ++j)
            {
                //For each sphere add a new btSphere to the bullet world (and add the displacement
                //specified by 'position')
                spherePositions.push_back(m_sceneObjects[i]->m_spherePack->getSphereAt(j) + m_sceneObjectPositions[i]);
                m_bullet->addSphere(m_sceneObjects[i]->m_spherePack->getSphereAt(j) + m_sceneObjectPositions[i], 1.0f, QVector3D(0,0,0), settings.radius);
            }

            //Mark how many spheres were added for this mesh
            m_sphereNumbers.push_back(sphereNum);

            m_drawSphereStates[i - 1] = settings.drawSpheres;

            //Emit a signal so that the number of spheres in the UI gets updated
            emit setSphereNumber(sphereNum);

            update();
            break;
        }
    }
}

void GLWidget::runConstraints(QString meshName, ConstraintSettings &settings)
{
    //Similar to the last two slots but for constraints
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->getName() == meshName.toStdString())
        {
            //Get the number of collision objects in the world now
            uint nBodies = m_bullet->getNumCollisionObjects();

            int constCount = 0;

            //A container for storing the constraints of the spheres by index
            std::vector< std::pair<uint, uint> > sphereConstraints;

            //A value for storing how many other bodies there are in the world,
            //the value starts at 1 because we know there is already the static
            //ground plane
            uint numOtherBodies = 1;

            //Iterate through this container and add the right number of spheres
            for(uint j = 0; j < m_sphereNumbers.size(); ++j)
            {
                //Ignore the object we're currently adding constraints to
                if(j == (i - 1))
                {
                    continue;
                }

                numOtherBodies += m_sphereNumbers[j];
            }

            //Iterate through the bodies in the 'world'
            for(uint j = numOtherBodies; j < nBodies; ++j)
            {
                //Create a new container for pairs of sphere indices
                std::vector< std::pair<uint, uint> > sphereIndices;

                //Mark the index of the sphere (accounting for any other objects
                //in the bullet world).
                uint sphereIndex = j - numOtherBodies;

                //Get the possible connections the current sphere could have in
                //the sphere pack
                std::vector<QVector3D> spheresToConnect;

                //See if the user has specified a maximum number of constraints or not
                if(settings.isMax)
                {
                    m_sceneObjects[i]->m_spherePack->getCloseSpheres(sphereIndex, spheresToConnect, sphereIndices, settings.maxNumber, settings.constraintStrength);
                }
                else
                {
                    //If they haven't set the maximum number ridiculously high (so it won't have any affect)
                    m_sceneObjects[i]->m_spherePack->getCloseSpheres(sphereIndex, spheresToConnect, sphereIndices, 1000000, settings.constraintStrength);
                }

                //Check if there are any candidate spheres to connect to
                if(spheresToConnect.size() == 0)
                {
                    //We can ignore this sphere if there aren't any
                    continue;
                }
                else
                {
                    //There are some spheres that can be constrained to
                    for(uint k = 0; k < sphereIndices.size(); ++k)
                    {
                        //Iterate through each one, check if the first sphere index
                        //in the container of pairs is the same as the current
                        //bullet rigid body we are testing
                        uint sphereA = sphereIndices[k].first;

                        if(sphereIndex == sphereA)
                        {
                            //If it is find out what the body is paired to
                            uint sphereB = sphereIndices[k].second;

                            sphereConstraints.push_back(std::make_pair(sphereA, sphereB));

                            //The indices stored are created from the list of spheres,
                            //because we may also have other objects in the
                            //bullet world the value needs to be adjusted
                            sphereA += numOtherBodies;
                            sphereB += numOtherBodies;

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

                            //I originally tried doing this the same way as with A
                            //but found that this caused the two bodies to fly toward
                            //each other. Instead I convert the frame of B so that it
                            //is relative to A.
                            transB = (bodyA->getCenterOfMassTransform() * transA) * (bodyB->getCenterOfMassTransform().inverse());

                            //Add the constraint to the bullet world using the specified type
                            if(settings.useFixed)
                            {
                                m_bullet->addFixedConstraint(bodyA, bodyB, transA, transB);
                            }
                            else if(settings.useSpring)
                            {
                               m_bullet->addSpringConstraint(bodyA, bodyB, transA, transB);
                            }

                            //Mark the addition of a new constraint
                            constCount++;
                        }
                    }
                }
            }

            //Add the constraints to a container so they can be draw later
            m_constraints.push_back(sphereConstraints);

            update();
            break;
        }
    }
}

void GLWidget::toggleDrawSpheres(QString meshName, bool drawSpheres)
{
    //Changes whether the given mesh is having it's spheres drawn or not
    for(uint i = 0; i < m_sceneObjects.size(); ++i)
    {
        if(m_sceneObjects[i]->getName() == meshName.toStdString() && m_sceneObjects[i]->getName().length() == meshName.toStdString().length())
        {
            m_drawSphereStates[i - 1] = drawSpheres;
        }
    }

    update();
}

void GLWidget::toggleDrawConstraints(QString meshName, bool drawConstraints)
{
    //Same as previous slot but for constraint drawing
    if(m_constraints.size() > 0)
    {
        for(uint i = 0; i < m_sceneObjects.size(); ++i)
        {
            if(m_sceneObjects[i]->getName() == meshName.toStdString())
            {
                m_drawConstraintStates[i - 1] = drawConstraints;
                update();
                break;
            }
        }
    }
}

void GLWidget::toggleSimulation(bool isSimulating)
{
    //Set simulating boolean according to the sent signal
    m_isSimulating = isSimulating;
}

void GLWidget::resetSimulation()
{
    //User has reset the simulation, so stop everything
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

    //Step the simulation once so that everything updates
    m_bullet->step(1.0f/60.0f, 10);

    //And stop the simulation (don't simulate any further)
    m_isSimulating = false;

    update();
}

void GLWidget::setSimulation(float stepValue)
{
    //Set the simulation step value
    m_simulationStep = stepValue;
}

void GLWidget::moveDown()
{
    //User has pressed the move down button
    if(m_moveUp)
    {
        //If we were already moving up, cancel that
        m_moveUp = false;
    }
    else
    {
        //Set the direction of movement and update
        //movement vector
        m_moveDown = true;
        m_adjustPos[1] -= 20.0f;
    }
}

void GLWidget::moveUp()
{
    //Essentially the opposite of the 'moveDown' slot
    if(m_moveDown)
    {
        m_moveDown = false;
    }
    else
    {
        m_moveUp = true;
        m_adjustPos[1] += 20.0f;
    }
}

void GLWidget::stopMove()
{
    //The user isn't pressing either move up or down button so
    //reset everything for future use
    m_adjustPos[1] = 0.0f;
    m_moveDown = false;
    m_moveUp = false;
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
     startTimer(1);

     //Create a ground plane in the scene objects so that
     //it gets drawn. A ground plane was already added to
     //the BtWorld in the constructor of this class
     createGround();

     //Create a sphere which will be used to draw the sphere pack. By doing this
     //there only ever has to be one instance of the sphere, it just gets drawn
     //in different positions and scaled accordingly in the shader to show different
     //bullet objects
     m_sphere.loadMesh("objFiles/sphere.obj");
     m_sphere.prepareMesh(m_pgm);

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
    m_pgm.setUniformValue("camPos", newCamPos);

    //This value is used for drawing spheres of different sizes (without having to store different meshes
    //or do anything procedurally
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

    //Create a new random colour for the mesh
    QVector4D colour((float)rand()/RAND_MAX,
                                (float)rand()/RAND_MAX,
                                (float)rand()/RAND_MAX,
                                1.0f);

    //Check if the mesh has actually already been added
    if(checkExisting(name, index))
    {
        qInfo()<<(QString)name.c_str()<<" has already been added, duplicating";
        std::shared_ptr<Mesh> mesh(new Mesh(*m_sceneObjects[index]));

        mesh->setColour(colour);
        //These two methods make sure the mesh VAOs which do not
        //have copy constructors/assignment operators (an so couldn't
        //be copied by the Mesh copy constructor) are created
        mesh->prepareMesh(m_pgm);

        //Add the mesh to the list of objects
        m_sceneObjects.push_back(mesh);

        //Mark its new position
        newMeshPosition = (int)m_sceneObjects.size() - 1;
    }
    else
    {
        //Create a new mesh shared pointer and use the colour constructor
        std::shared_ptr<Mesh> mesh(new Mesh(colour, name));

        //Load the teapot obj
        mesh->loadMesh(filepath);

        //Load the neccesary vaos and vbos
        mesh->prepareMesh(m_pgm);

        //Add the pointer to the vector of scene objects
        m_sceneObjects.push_back(mesh);

        //And store the position
        newMeshPosition = (int)m_sceneObjects.size() - 1;
    }

    m_meshSkinStates.push_back(false);

    //Finally add the desired position of the mesh to a container
    m_sceneObjectPositions.push_back(position);

    m_drawMeshStates.push_back(true);
    m_drawSphereStates.push_back(false);
    m_drawConstraintStates.push_back(false);

    //Emit a signal which will change the colour in the Shader tab of the UI
    emit setMeshColour(colour);

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
        if(m_drawConstraintStates[i])
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

    //Check there are objects in the scene (no need to do the following
    //steps if not. Notice size > 1 as we are ignoring the ground plane
    if(m_sceneObjects.size() > 1)
    {
        //Get the number of bullet bodies
        uint nBodies = m_bullet->getNumCollisionObjects();

        //A container used to store the positions of the spheres
        //in the sphere pack
        vector_V spherePositions;

        //Two counters, one for the number of bodies/meshes in
        //the scene and the other for the number of spheres
        int numSpheres = 0;
        int bodyNumber = 0;

        //Clear the position of all the spheres
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
                if(m_sphereNumbers.size() > 0)
                {
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
        }

        //Iterate through all the objects in the mesh
        int sphereIndex = 0;
        for(uint i = 1; i < m_sceneObjects.size(); ++i)
        {
            if(m_meshSkinStates[i - 1])
            {
                //Update the skinned mesh
                m_sceneObjects[i]->updateSkinnedMesh(m_spherePositions[sphereIndex]);

                //Update the skinned mesh VAO for drawing later
                m_sceneObjects[i]->prepareSkinnedMesh(m_pgm);

                sphereIndex++;
            }
        }

        //Set the drawing colour to the sphere colour
        m_pgm.setUniformValue("mCol",m_sphere.getColour());

        //Are we currently drawing the spheres or not?
        if(m_drawSphereStates.size() > 0)
        {
            for(uint i = 1; i < m_sceneObjects.size(); ++i)
            {
                if(m_drawSphereStates[i - 1])
                {
                    //We are, iterate through the 2x2 sphere position
                    //matrix
                    for(uint k = 0; k < m_spherePositions[i - 1].size(); ++k)
                    {
                        //Set the drawing position accordingly and load
                        //it to the shader. Make sure to set the drawing
                        //scale to the radius (the sphere mesh has a
                        //radius of one to facilitate this).
                        m_position = m_spherePositions[i - 1][k];

                        loadShaderMatrices(m_sphereRadii[i - 1]);

                        //Finally draw the instance of the sphere that was
                        //created in initializeGL()
                        m_sphere.draw();
                    }
                }
            }
        }

        //Reset the drawing position
        m_position = QVector3D(0,0,0);

        //Reload it with a drawing scale of 1
        loadShaderMatrices(1.0f);

        if(m_drawConstraintStates.size() > 0)
        {
            for(uint i = 1; i < m_sceneObjects.size(); ++i)
            {
                if(m_drawConstraintStates[i - 1])
                {
                    //Draw constraints (if we're meant to);
                    updateConstraintDrawing();

                    //The vao will only contain data from constraints
                    //which are due to be drawn, so we can break afterwards
                    m_vaoConstraint.bind();
                    m_pgm.setUniformValue("mCol",m_sphere.getColour());

                    glLineWidth(0.25f);
                    glDrawArrays(GL_LINES, 0, (int)constraintVerts.size());
                    m_vaoConstraint.release();
                    break;
                }
            }
        }

        //Are we drawing any meshes?
        if(m_drawMeshStates.size() > 1)
        {
            //We are, iterate through the objects in the scene starting
            //at 1 because 0 (the ground plane) will be done later
            for(uint i = 1; i < m_sceneObjects.size(); ++i)
            {
                if(m_drawMeshStates[i])
                {
                    //Set the right colour
                    m_pgm.setUniformValue("mCol",m_sceneObjects[i]->getColour());

                    //Check for skinned verts
                    if(m_sceneObjects[i]->isSkinned())
                    {
                        //Skinned verts will already have position so
                        //the drawing position doesn't need updating
                        m_sceneObjects[i]->draw();
                    }
                    else
                    {
                        //Otherwise we need to use the position specified when
                        //the mesh was added to the scene
                        m_position = m_sceneObjectPositions[i];

                        loadShaderMatrices(1.0f);

                        m_sceneObjects[i]->draw();
                    }
                }
            }
        }
    }

    //Finally draw the ground plane, as this is always visible
    //it can be put outside of the if statement

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

void GLWidget::qNormalizeAngle(int &angle)
{
    //This uses the mouse/camera movement example on Qt's website:
    //http://doc.qt.io/qt-4.8/qt-opengl-hellogl-example.html

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
    //QTimerEvent isn't used

    //Check if the program should be simulating
    if(m_isSimulating)
    {
        //Make sure bodies are moved if they're meant to be
        if(m_moveUp)
        {
            m_bullet->moveBodies(m_adjustPos);
        }
        else if(m_moveDown)
        {
            m_bullet->moveBodies(m_adjustPos);
        }

        //Step the simulation with a very high max substeps
        //to account for simulation step changes
        m_bullet->step(m_simulationStep, 20);

        //Update the window
        update();
    }
}
