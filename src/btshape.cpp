#include "btshape.h"

//Create an instance of this class but set it to NULL for now
BtShape* BtShape::s_instance = NULL;

BtShape *BtShape::instance()
{
    //Test if the instance already exists
    if(s_instance == NULL)
    {
        //If it doesn't then create one
        s_instance = new BtShape;
    }

    //Return it
    return s_instance;
}

void BtShape::destroy()
{
    //Delete the data in s_instance and reset it to NULL
    delete s_instance;
    s_instance = NULL;
}

//CTor and DTor are private
BtShape::BtShape()
{

}

BtShape::~BtShape()
{

}

void BtShape::addMesh(const std::string name, const char* filepath, QVector3D colour)
{
    //Addthe mesh using the given data
    Mesh mesh(QVector4D(colour, 1.0f), name);

    //Load it
    mesh.loadMesh(filepath);

    //Create a new convex shape for the mesh
    btConvexHullShape* shape = new btConvexHullShape();

    //Get the verts of the mesh
    auto verts = mesh.getVerts();

    for(QVector3D vert : verts)
    {
        //Add the verts one by one to the convex shape
        shape->addPoint(btVector3(vert.x(),vert.y(),vert.z()));
    }

    //Add the shape to the stored shapes
    m_shapes[name].reset(shape);
}

void BtShape::addSphere(const std::string name, float radius)
{
    //Much easier than add mesh, create a new btSphereShape
    //and then add it to the map
    btCollisionShape* shape = new btSphereShape(btScalar(radius));

    m_shapes[name].reset(shape);
}

btCollisionShape* BtShape::getShape(const std::string name)
{
    //Create a new shape but set it to NULL
    btCollisionShape *shape = NULL;

    //Return a horrible iterator type so make it auto
    auto foundShape = m_shapes.find(name);

    //If the iterator isn't at the end of the map then
    //the shape was found (so store it)
    if(foundShape != m_shapes.end())
    {
        shape = foundShape->second.get();
    }

    //Finally return the shape. Generally returning a NULL
    //is bad but I have ensured that where this function is
    //called a NULL case is handled
    return shape;
}
