#include "btworld.h"

#include <iostream>

BtWorld::BtWorld()
{
    m_collisionConfig.reset(new btDefaultCollisionConfiguration());

    m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfig.get()));

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
}

void BtWorld::addSphere(const QVector3D &pos, float mass, const QVector3D &inertia)
{
    btCollisionShape* sphere = BtShape::instance()->getShape("sphere");

    btTransform startPos;
    startPos.setIdentity();

    btScalar m(mass);

    btVector3 sphereInertia(inertia.x(), inertia.y(), inertia.z());
    sphere->calculateLocalInertia(m, sphereInertia);
    startPos.setOrigin(btVector3(pos.x(), pos.y(), pos.z()));

    btDefaultMotionState* sphereMotionState = new btDefaultMotionState(startPos);
    btRigidBody::btRigidBodyConstructionInfo sphereCI(m, sphereMotionState, sphere, sphereInertia);
    btRigidBody* body = new btRigidBody(sphereCI);

    body->setFriction(1.0);
    body->setRollingFriction(1.0);

    m_dynamicsWorld->addRigidBody(body);
    Body rbSphere;
    rbSphere.name = "sphere";
    rbSphere.body = body;

    m_bodies.push_back(rbSphere);
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
