#ifndef BTSHAPE_H
#define BTSHAPE_H

#include <btBulletDynamicsCommon.h>

#include <unordered_map>
#include <memory>
#include <string>

#include "mesh.h"

//Worth noting this class isn't used but I implemented it initially so that meshes
//could easily be added to the BtWorld and referred to by name. To that end
//I have left it in in case I want to improve the project post-deadline
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
