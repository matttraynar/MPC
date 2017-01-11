#ifndef BTWORLD_H
#define BTWORLD_H

//Bullet library includes (order is important)
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <QVector3D>

#include <memory>

#include "btshape.h"

class BtWorld
{
public:
    BtWorld();
    ~BtWorld();

    //A few functions for settings up, starting, stopping, resetting and running
    //bullet physics
    void setGravity(float x, float
                    y, float z);
    void step(float time, float step);
    void stop();
    void reset(QVector3D position, uint index);
    void remove(uint index);
    void removeBodies(uint index, uint size);

    //These are used to add the default static ground plane and add any spheres
    void addGround();
    void addSphere(const QVector3D &pos, float mass, const QVector3D &inertia, float radius);

    //This function is similar to BtShape in that it is not used but it useful in case I want
    //to add mesh functionality to the program at a later date
    void addMesh(const std::string name, QVector3D pos);

    //Two methods for constraints. Fairly self explanatory
    void addFixedConstraint(btRigidBody* bodyA, btRigidBody* bodyB, btTransform transformA, btTransform transformB);
    void addSpringConstraint(btRigidBody* bodyA, btRigidBody* bodyB, btTransform transformA, btTransform transformB);

    //Used in various places by other classes in order to get information from the bullet world
    uint getNumCollisionObjects() const;
    QVector3D getTransform(uint index) const;

    //These functions enable me to easily get rigid bodies and also refer
    //to them by 'name'
    inline std::string getBodyNameAt(uint i) const { return m_bodies[i].name; }
    inline btRigidBody* getBodyAt(uint i) const { return m_bodies[i].body.get(); }

    //Simply function which can move all of the dynamic objects in the scene by the given vector
    void moveBodies(QVector3D moveVec);
    void stopAdjusting();

private:
    //The various solvers etc need to make bullet work
    btDefaultCollisionConfiguration*               m_collisionConfig;
    btCollisionDispatcher*                            m_dispatcher;
    btDbvtBroadphase*                                 m_broadphase;
    btSequentialImpulseConstraintSolver*      m_solver;
    btDiscreteDynamicsWorld*                     m_dynamicsWorld;

    //The ground plane
    std::shared_ptr<btCollisionShape> m_groundPlane;

    //A structure which allows me to refer to bullet rigid bodies by a name
    typedef struct
    {
      std::string name;
      std::shared_ptr<btRigidBody> body;

    }Body;

    //A container for the above structure
    std::vector<Body> m_bodies;
};

#endif // BTWORLD_H
