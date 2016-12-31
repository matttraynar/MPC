#include "btworld.h"

#include <iostream>

BtWorld::BtWorld()
{
    m_collisionConfig.reset(new btDefaultCollisionConfiguration());

    m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfig.get()));    

    btGImpactCollisionAlgorithm::registerAlgorithm(new btCollisionDispatcher(m_collisionConfig.get())   );

    m_broadphase.reset(new btDbvtBroadphase());

    m_solver.reset(new btSequentialImpulseConstraintSolver);

    m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(m_dispatcher.get(),
                                                      m_broadphase.get(),
                                                      m_solver.get(),
                                                      m_collisionConfig.get()));
}

void BtWorld::setGravity(float x, float y, float z)
{
    m_dynamicsWorld->setGravity(btVector3(x,y,z));
}

void BtWorld::step(float time, float step)
{
    m_dynamicsWorld->stepSimulation(time,step);
}

void BtWorld::stop()
{
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
    btTransform resetPosition(btQuaternion::getIdentity(), btVector3(position.x(), position.y() + 5, position.z()));
    m_bodies[index].body->setWorldTransform(resetPosition);
}

void BtWorld::addGround()
{
    m_groundPlane.reset(new btStaticPlaneShape(btVector3(0.0f, 1.0f, 0.0f), 0.0f));

    btTransform groundTrans;
    groundTrans.setIdentity();

    btDefaultMotionState* groundMotion = new btDefaultMotionState(groundTrans);
    btRigidBody::btRigidBodyConstructionInfo groundCI(0.0, groundMotion, m_groundPlane.get());

    btRigidBody* groundRB = new btRigidBody(groundCI);
    groundRB->setFriction(1.0);
    groundRB->setRollingFriction(2.0);

    m_dynamicsWorld->addRigidBody(groundRB);

    Body ground;
    ground.name = "groundPlane";
    ground.body = groundRB;

    m_bodies.push_back(ground);

    groundRB->setUserPointer(&m_bodies[m_bodies.size() - 1]);
}

void BtWorld::addMesh(const std::string name, QVector3D pos)
{
    btCollisionShape* newMesh = BtShape::instance()->getShape(name);

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
    mesh.body = meshRB;
    m_bodies.push_back(mesh);

    meshRB->setUserPointer(&m_bodies[m_bodies.size() - 1]);
}

void BtWorld::addSphere(const QVector3D &pos, float mass, const QVector3D &inertia)
{
    btCollisionShape* newSphere = BtShape::instance()->getShape("sphere");

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

//    body->setDamping(btScalar(0.5f), btScalar(0.5f));

    m_dynamicsWorld->addRigidBody(sphereRB);
    Body sphere;
    sphere.name = "sphere";
    sphere.body = sphereRB;

    m_bodies.push_back(sphere);

    sphereRB->setUserPointer(&m_bodies[m_bodies.size() - 1]);
}

void BtWorld::addFixedConstraint(btRigidBody* bodyA, btRigidBody* bodyB, btTransform transformA, btTransform transformB)
{
    //Create a new fixed constraint from the given data
    btFixedConstraint* connection = new btFixedConstraint(*bodyA, *bodyB, transformA, transformB);
    connection->enableFeedback(true);

    //Add the constraint to the world
    m_dynamicsWorld->addConstraint(connection, false);

    m_constraints.push_back(connection);
}

void BtWorld::checkVelocities()
{
    uint nBodies = m_dynamicsWorld->getNumCollisionObjects();

    btScalar maxSpeed = 5.0f;

    //Iterate through them
    for(uint i = 0; i < nBodies; ++i)
    {
        btRigidBody* body = m_bodies[i].body;

        btVector3 bodyVel = body->getLinearVelocity();
        btScalar bodySpd  = bodyVel.length();

        if(bodySpd > maxSpeed)
        {
            bodyVel *= maxSpeed/bodySpd;

            body->setLinearVelocity(bodyVel);
        }
    }
}

void BtWorld::checkPlastic()
{
    uint nConstraints = m_constraints.size();

    //Iterate through them
    for(uint i = 0; i < nConstraints; ++i)
    {
        btTypedConstraint* currentConstraint = m_constraints[i];
        btScalar appliedImpulse = currentConstraint->getAppliedImpulse();

        if(appliedImpulse > 0.8f)
        {
            qInfo()<<"Impulse too high";
            btRigidBody* bodyA = &(currentConstraint->getRigidBodyA());
            btRigidBody* bodyB = &(currentConstraint->getRigidBodyB());


            m_dynamicsWorld->removeConstraint(m_constraints[i]);

            btTransform transA;
            transA.setIdentity();
            bodyA->getMotionState()->getWorldTransform(transA);

            btTransform transB;
            transB.setIdentity();

            transB = (bodyA->getCenterOfMassTransform() * transA) * (bodyB->getCenterOfMassTransform().inverse());

            addFixedConstraint(bodyA, bodyB, transA, transB);
        }
    }
}

uint BtWorld::getNumCollisionObjects() const
{
    return m_dynamicsWorld->getNumCollisionObjects();
}

QVector3D BtWorld::getTransform(uint index) const
{
    btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[index];
    btRigidBody* rBody = btRigidBody::upcast(obj);

    if(rBody && rBody->getMotionState())
    {
        btTransform bodyTrans;

        rBody->getMotionState()->getWorldTransform(bodyTrans);

        float matrix[16];
        bodyTrans.getOpenGLMatrix(matrix);


        btVector3 origin = bodyTrans.getOrigin();

        return QVector3D(origin.x(),origin.y(),origin.z());
    }
    else
    {
        return QVector3D(0.0f, 0.0f, 0.0f);
    }

}

void BtWorld::moveBodies(QVector3D moveVec)
{
    uint nBodies = m_dynamicsWorld->getNumCollisionObjects();

    //Iterate through them
    for(uint i = 0; i < nBodies; ++i)
    {
        btRigidBody* body = m_bodies[i].body;

        body->setLinearVelocity(btVector3(moveVec.x(), moveVec.y(), moveVec.z()));
    }
}

void BtWorld::stopAdjusting()
{
    uint nBodies = m_dynamicsWorld->getNumCollisionObjects();

    //Iterate through them
    for(uint i = 0; i < nBodies; ++i)
    {
        btRigidBody* body = m_bodies[i].body;

        body->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
    }
}
