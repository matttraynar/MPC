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

void BtWorld::addConstraint(btTypedConstraint* constraint)
{
    m_dynamicsWorld->addConstraint(constraint);
    m_numConstraints++;
}

void BtWorld::addFixedConstraint(btRigidBody* bodyA, btRigidBody* bodyB, btTransform transformA, btTransform transformB)
{
    btFixedConstraint* connection = new btFixedConstraint(*bodyA, *bodyB, transformA, transformB);

    connection->setLinearLowerLimit(btVector3(0,0,0));
    connection->setLinearUpperLimit(btVector3(1,1,1));
    connection->setAngularLowerLimit(btVector3( 0, 0, 0 ));
    connection->setAngularUpperLimit(btVector3( 0, 0, 0 ));


    m_dynamicsWorld->addConstraint(connection, false);

    m_constraints.push_back(connection);

//    Body* constrainFrom = (Body*)(bodyA->getUserPointer());
//    Body* constrainTo = (Body*)(bodyB->getUserPointer());

//    constrainFrom->constraints.push_back(connection);
//    constrainTo->constraints.push_back(connection);

    m_numConstraints++;

//    connection->setEnabled(false);

    if(connection->isEnabled())
    {
        qInfo()<<"Constraint enabled";
    }
    else
    {
        qInfo()<<"Constraint not enabled";
    }

}

void BtWorld::removeConstraint(btTypedConstraint* constraint)
{
    m_dynamicsWorld->removeConstraint(constraint);
    m_numConstraints--;
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

void BtWorld::checkCollisions()
{
    int numManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();

    for(int i = 0; i < numManifolds; ++i)
    {
        btPersistentManifold* contactMan = m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

        const btCollisionObject* objA = static_cast<const btCollisionObject*>(contactMan->getBody0());
        const btCollisionObject* objB = static_cast<const btCollisionObject*>(contactMan->getBody1());

        int numContacts = contactMan->getNumContacts();

        Body* thisObj = (Body*)(objA->getUserPointer());
        Body* thatObj = (Body*)(objB->getUserPointer());

        if(thisObj == NULL || thatObj == NULL)
        {
            qInfo()<<"Pointer was null";
            continue;
        }

        btRigidBody* thisRBody= thisObj->body;
        btRigidBody* thatRBody = thatObj->body;

        if(thisRBody == NULL || thatRBody == NULL)
        {
            qInfo()<<"Pointer was null";
            continue;
        }

        Body* thisRObj = (Body*)(thisRBody->getUserPointer());
        Body* thatRObj = (Body*)(thatRBody->getUserPointer());




        qInfo()<<"name "<<QString(thisRObj->name.c_str());
        qInfo()<<"name "<<QString(thatRObj->name.c_str());

//        for(int j = 0; j < numContacts; ++j)
//        {
//            btManifoldPoint& point = contactMan->getContactPoint(j);

//            if(point.getDistance() < 0.0f)
//            {
//                //Get the stored pointer from the collision object. Because this returns a void*
//                //I have to convert it to a shared_ptr* and then dereference it


//                if(thisObj == NULL || thatObj == NULL)
//                {
//                    qInfo()<<"Pointer was null";
//                }
//                else if(strcmp(thatObj->name.c_str(), "groundPlane"))
//                {
//                    qInfo()<<"Sphere collision that=ground";

//                    int thisObjConstraintCount = thisObj->constraints.size();


//                    qInfo()<<"Num constraints: "<<thisObjConstraintCount;

//                    for(int k = 0; k < thisObjConstraintCount; ++k)
//                    {
//                        thisObj->constraints[k]->setEnabled(true);
//                    }
//                }
//                else if(strcmp(thisObj->name.c_str(), "groundPlane"))
//                {
//                    qInfo()<<"Sphere collision this=ground";

//                    int thatObjConstraintCount = thatObj->constraints.size();

//                    for(int k = 0; k < thatObjConstraintCount; ++k)
//                    {
//                        thatObj->constraints[k]->setEnabled(true);
//                    }
//                }
//            }
//        }
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
