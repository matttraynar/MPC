#include "btshape.h"

BtShape::BtShape()
{

}

BtShape *BtShape::instance()
{
    static BtShape s_instance;

    return &s_instance;
}

BtShape::~BtShape()
{

}

void BtShape::addMesh(const std::string name, const char* filepath, QVector3D colour)
{
    Mesh mesh;
    mesh.loadMesh(filepath);
    mesh.setColour(colour);

    btConvexHullShape* shape = new btConvexHullShape();
    auto verts = mesh.getVerts();

    for(QVector3D vert : verts)
    {
        shape->addPoint(btVector3(vert.x(),vert.y(),vert.z()));
    }

    m_shapes[name].reset(shape);

}

void BtShape::addSphere(const std::string name, float radius)
{
    btCollisionShape* shape = new btSphereShape(btScalar(radius));

    m_shapes[name].reset(shape);
}

btCollisionShape* BtShape::getShape(const std::string name)
{
    btCollisionShape *shape = nullptr;

    auto foundShape = m_shapes.find(name);

    if(foundShape != m_shapes.end())
    {
        shape = foundShape->second.get();
    }

    return shape;
}
