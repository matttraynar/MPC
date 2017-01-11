#ifndef BTWORLD_H
#define BTWORLD_H

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include <QVector3D>

#include <memory>

#include "btshape.h"

class BtWorld
{
public:
    BtWorld();
    ~BtWorld();

    void setGravity(float x, float
                    y, float z);
    void step(float time, float step);
    void stop();
    void reset(QVector3D position, uint index);
    void remove(uint index);
    void removeBodies(uint index, uint size);

    void addGround();
    void addMesh(const std::string name, QVector3D pos);
    void addSphere(const QVector3D &pos, float mass, const QVector3D &inertia, float radius);

    void addFixedConstraint(btRigidBody* bodyA, btRigidBody* bodyB, btTransform transformA, btTransform transformB);
    void addSpringConstraint(btRigidBody* bodyA, btRigidBody* bodyB, btTransform transformA, btTransform transformB);

    uint getNumCollisionObjects() const;
    QVector3D getTransform(uint index) const;

    //These functions enable me to easily get rigid bodies and also refer
    //to them by 'name'
    inline std::string getBodyNameAt(uint i) const { return m_bodies[i].name; }
    inline btRigidBody* getBodyAt(uint i) const { return m_bodies[i].body.get(); }

    void moveBodies(QVector3D moveVec);
    void stopAdjusting();

private:
    btDefaultCollisionConfiguration*               m_collisionConfig;
    btCollisionDispatcher*                            m_dispatcher;
    btDbvtBroadphase*                                 m_broadphase;
    btSequentialImpulseConstraintSolver*      m_solver;
    btDiscreteDynamicsWorld*                     m_dynamicsWorld;

    std::shared_ptr<btCollisionShape> m_groundPlane;

    typedef struct
    {
      std::string name;
      std::shared_ptr<btRigidBody> body;

    }Body;

    std::vector<Body> m_bodies;
};

#endif // BTWORLD_H
