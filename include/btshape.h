#ifndef BTSHAPE_H
#define BTSHAPE_H

#include <btBulletDynamicsCommon.h>

#include <unordered_map>
#include <memory>
#include <string>

#include "mesh.h"

class BtShape
{
public:
    static BtShape* instance();
    ~BtShape();

    void addMesh(const std::string name, const char* filepath, QVector3D colour);
    void addSphere(const std::string name, float radius);

    btCollisionShape* getShape(const std::string name);

private:
    BtShape();

    std::unordered_map< std::string, std::unique_ptr<btCollisionShape> > m_shapes;

};

#endif // BTSHAPE_H
