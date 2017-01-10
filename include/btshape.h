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
    static void destroy();

    void addMesh(const std::string name, const char* filepath, QVector3D colour);
    void addSphere(const std::string name, float radius);

    btCollisionShape* getShape(const std::string name);    

private:
    BtShape();
    ~BtShape();

    std::unordered_map< std::string, std::shared_ptr<btCollisionShape> > m_shapes;

    static BtShape *s_instance;
};

#endif // BTSHAPE_H
