#include "btworld.h"

#include <iostream>

BtWorld::BtWorld()
{
    //Set up the bullet world using the normal settings. It
    //would addmitedly be nicer to do this with smart pointers
    //but this program doesn't have ownership so can' be done
    m_collisionConfig = new btDefaultCollisionConfiguration();

    m_dispatcher = new btCollisionDispatcher(m_collisionConfig);

    m_broadphase = new btDbvtBroadphase();

    m_solver = new btSequentialImpulseConstraintSolver();

    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,
                                                      m_broadphase,
                                                      m_solver,
                                                      m_collisionConfig);
}

BtWorld::~BtWorld()
{
    for(int i = 0; i < m_bodies.size(); ++i)
    {
        m_dynamicsWorld->removeRigidBody(m_bodies[i].body.get());
    }

    delete m_collisionConfig;
    delete m_dispatcher;
    delete m_broadphase;
    delete m_solver;
    delete m_dynamicsWorld;
}

void BtWorld::setGravity(float x, float y, float z)
{
    //Set the world's gravity accordingly
    m_dynamicsWorld->setGravity(btVector3(x,y,z));
}

void BtWorld::step(float time, float step)
{
    //Step the simulation forward
//    m_dynamicsWorld->stepSimulation(time,step);
    m_dynamicsWorld->stepSimulation(btScalar(time),int(step));
}

void BtWorld::stop()
{
    //Stop the simulation by iterating through each
    //body and clearing all forces
    uint bodies = getNumCollisionObjects();

    for(uint i = 0; i < bodies; ++i)
    {
        m_bodies[i].body->clearForces();
        m_bodies[i].body->setLinearVelocity(btVector3(0,0,0));
        m_bodies[i].body->setAngularVelocity(btVector3(0,0,0));
    }
}

void BtWorld::reset(QVector3D position, uint index)
{
    //Reset the simulation by moving everything back to where it started
    btTransform resetPosition(btQuaternion::getIdentity(), btVector3(position.x(), position.y(), position.z()));
    m_bodies[index].body->setWorldTransform(resetPosition);
}

void BtWorld::remove(uint index)
{
    m_dynamicsWorld->removeRigidBody(m_bodies[index].body.get());
}

void BtWorld::removeBodies(uint index, uint size)
{
    uint removed = 0;

    while(removed != (size - 1))
    {
        if(removed > 1000000)
        {
            break;
        }

        m_bodies.erase(m_bodies.begin() + index);
        removed++;
    }
}

void BtWorld::addGround()
{
    //Create a new ground plane and store it in the member variable
    m_groundPlane.reset(new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), 0.0f));

    //Set up the transformation and construction info for the new rigid body
    btTransform groundTrans;
    groundTrans.setIdentity();

    btDefaultMotionState* groundMotion = new btDefaultMotionState(groundTrans);
    btRigidBody::btRigidBodyConstructionInfo groundCI(0.0, groundMotion, m_groundPlane.get());

    //Create a rigid body for the ground. Wanted to do this using smart
    //pointers but because of some ownership issues this just
    //causes the program to crash.
    btRigidBody* groundRB = new btRigidBody(groundCI);
    groundRB->setFriction(1.0);
    groundRB->setRollingFriction(2.0);

    //Add the body to the bullet world
    m_dynamicsWorld->addRigidBody(groundRB);

    //Finally create a new 'body' (my own class)
    Body ground;
    ground.name = "groundPlane";
    ground.body.reset(groundRB);// = groundRB;

    //Store the body in a container so it can be accessed later
    m_bodies.push_back(ground);
}

void BtWorld::addMesh(const std::string name, QVector3D pos)
{
    //Add a new mesh collision shape using the stored shapes in BtShape
    btCollisionShape* newMesh = BtShape::instance()->getShape(name);

    //Check if it returned properly or not
    if(newMesh == NULL)
    {
        qInfo()<<"You haven't added"<<(QString)name.c_str()<<"to the shapes list so this mesh"
                                                             "cannot be added to the physics world";
        return;
    }

    //Everything else is much the same as with adding the ground plane
    btTransform meshTrans;
    meshTrans.setIdentity();

    btScalar mass(5.0);
    btVector3 inertia(10, 10, 10);

    newMesh->calculateLocalInertia(mass, inertia);
    meshTrans.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));

    btDefaultMotionState* meshMotion = new btDefaultMotionState(meshTrans);
    btRigidBody::btRigidBodyConstructionInfo meshCI(mass, meshMotion, newMesh, inertia);
    btRigidBody* meshRB = new btRigidBody(meshCI);

    meshRB->setFriction(1.0);
    meshRB->setRollingFriction(1.0);

    m_dynamicsWorld->addRigidBody(meshRB);

    Body mesh;
    mesh.name = name;
    mesh.body.reset(meshRB);// = meshRB;
    m_bodies.push_back(mesh);
}

void BtWorld::addSphere(const QVector3D &pos, float mass, const QVector3D &inertia, float radius)
{
    //This is the same as the addMesh function, only here the shape we want
    //from BtShape is known
    btCollisionShape* newSphere = new btSphereShape(btScalar(radius));

    btTransform startPos;
    startPos.setIdentity();

    btScalar m(mass);

    btVector3 sphereInertia(inertia.x(), inertia.y(), inertia.z());
    newSphere->calculateLocalInertia(m, sphereInertia);
    startPos.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));

    btDefaultMotionState* sphereMotionState = new btDefaultMotionState(startPos);
    btRigidBody::btRigidBodyConstructionInfo sphereCI(m, sphereMotionState, newSphere, sphereInertia);
    btRigidBody* sphereRB = new btRigidBody(sphereCI);

    sphereRB->setFriction(1.0);
    sphereRB->setRollingFriction(1.0);

    m_dynamicsWorld->addRigidBody(sphereRB);
    Body sphere;
    sphere.name = "sphere";
    sphere.body.reset(sphereRB);//= sphereRB;

    m_bodies.push_back(sphere);
}

void BtWorld::addFixedConstraint(btRigidBody* bodyA, btRigidBody* bodyB, btTransform transformA, btTransform transformB)
{
    //Create a new fixed constraint from the given data
    btFixedConstraint* connection = new btFixedConstraint(*bodyA, *bodyB, transformA, transformB);

    //Add the constraint to the world
    m_dynamicsWorld->addConstraint(connection, false);
}

uint BtWorld::getNumCollisionObjects() const
{
    //Pretty simply function but avoids chaining
    return m_dynamicsWorld->getNumCollisionObjects();
}

QVector3D BtWorld::getTransform(uint index) const
{
    //Again it would be nice to use smart pointers here but bullet doesn't
    //'allow' it
    btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[index];
    btRigidBody* rBody = btRigidBody::upcast(obj);

    //Check if the rigid body exists and is moving
    if(rBody && rBody->getMotionState())
    {
        //Create a new transform and store the objects
        //movement in there
        btTransform bodyTrans;
        rBody->getMotionState()->getWorldTransform(bodyTrans);

        //Get the position from the transform
        btVector3 origin = bodyTrans.getOrigin();

        //Return it as a QVector3D
        return QVector3D(origin.x(),origin.y(),origin.z());
    }
    else
    {
        //The object isn't moving, return a 0 vector
        return QVector3D(0.0f, 0.0f, 0.0f);
    }

}

void BtWorld::moveBodies(QVector3D moveVec)
{
    //Iterate through the bodies in the world
    for(uint i = 0; i < (uint)m_dynamicsWorld->getNumCollisionObjects(); ++i)
    {
        //Get each body and set its velocity using
        //the movement vector
        btRigidBody* body = m_bodies[i].body.get();
        body->setLinearVelocity(btVector3(moveVec.x(), moveVec.y(), moveVec.z()));
    }
}

void BtWorld::stopAdjusting()
{
    //Iterate through the bodies
    for(uint i = 0; i < (uint)m_dynamicsWorld->getNumCollisionObjects(); ++i)
    {
        //Set the velocity of each body to 0
        btRigidBody* body = m_bodies[i].body.get();

        body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    }
}
