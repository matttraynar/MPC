#ifndef BTWORLD_H
#define BTWORLD_H

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <QVector3D>

#include <memory>

#include "btshape.h"

class BtWorld
{
public:
    BtWorld();

    void setGravity(float x, float y, float z);
    void step(float time, float step);

    void addGround();
    void addMesh(const std::string name, QVector3D pos);
    void addSphere(const QVector3D &pos, float mass, const QVector3D &inertia);

    void addConstraint(btTypedConstraint *constraint);

    uint getNumCollisionObjects() const;
    QVector3D getTransform(uint index) const;
    inline std::string getBodyNameAt(uint i) const { return m_bodies[i].name; }
    inline btRigidBody* getBodyAt(uint i) const { return m_bodies[i].body; }

private:
    std::unique_ptr<btDefaultCollisionConfiguration>     m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher>               m_dispatcher;
    std::unique_ptr<btDbvtBroadphase>                    m_broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr< btDiscreteDynamicsWorld>            m_dynamicsWorld;

    std::shared_ptr<btCollisionShape> m_groundPlane;

    typedef struct
    {
      std::string name;
      btRigidBody* body;

    }Body;

    std::vector<Body> m_bodies;
};

#endif // BTWORLD_H
