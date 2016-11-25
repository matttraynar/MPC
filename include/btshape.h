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
    static BtShape *instance();

    void addMesh(const std::string name, const char* filepath, QVector3D colour);

    btCollisionShape* getShape(const std::string name);
    QVector3D* getColour(const std::string name);

private:
    BtShape();

    std::unordered_map< std::string, std::unique_ptr<btCollisionShape> > m_shapes;

    std::unordered_map< std::string, std::unique_ptr<QVector3D> >m_colours;

};

#endif // BTSHAPE_H
