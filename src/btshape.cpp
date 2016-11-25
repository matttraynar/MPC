#include "btshape.h"

BtShape::BtShape()
{

}

BtShape *BtShape::instance()
{
    static BtShape s_instance;

    return &s_instance;
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
    m_colours[name].reset(&colour);

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

QVector3D* BtShape::getColour(const std::string name)
{
    QVector3D* colour = nullptr;

    auto foundColour = m_colours.find(name);

    if(foundColour != m_colours.end())
    {
        colour = foundColour->second.get();
    }

    return colour;
}
