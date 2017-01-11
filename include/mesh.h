#ifndef MESH_H_
#define MESH_H_

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QtGui>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QVector3D>

#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <memory>

#include "spherepack.h"

//A structure used to store the boolean overload which
//will allow me to find a QVector in an std::vector
struct FindVector
{
    FindVector(QVector3D vec) : m_toFind(vec) {}

    QVector3D m_toFind;

    bool operator() (const QVector3D &i)
    {
        return((i.x() == m_toFind.x()) &&
                 (i.y() == m_toFind.y()) &&
                 (i.z() == m_toFind.z()));
    }
};

//Similar to the last struct, only this one is for sorting uint/float pairs
//by their second value
struct SortPair
{
    bool operator() (const std::pair<uint, float> &i, const std::pair<uint, float> &j)
    {
        return(i.second < j.second);
    }
};

class Mesh
{

public:
    //Ctors
    Mesh(QVector4D colour, const std::string name);
    Mesh(const Mesh &copy);

    //Dtor
    ~Mesh();

    //Methods for loading, passing to the shader and drawing the mesh
    void loadMesh(const char* filepath);
    void prepareMesh(QOpenGLShaderProgram &program);
    void prepareSkinnedMesh(QOpenGLShaderProgram &program);
    void draw();

    //Method which creates a sphere pack for the mesh (calls functions from
    //the 'spherepack' class
    void generateDistanceField(DistanceFieldSettings &settings);
    void runSpherePackAlgorithm(SpherePackSettings &settings);

    //Skins the mesh to the sphere in the sphere pack
    void skinMeshToSpheres(uint numControlSpheres);

    //Updates the positions of the skinned mesh verts relative to
    //the spheres they are skinned to
    void updateSkinnedMesh(const vector_V &spherePositions);

    //Set methods
    void setWireMode(bool wiremode);
    void setColour(QVector4D colour);
    void setName(const std::string name) { m_name = name;}

    //Get methods
    inline vector_V getVerts() const { return m_verts; }
    inline vector_V getSkinnedVerts() const { return m_skinnedVerts; }

    inline QVector4D getColour() const                  { return m_colour; }
    inline std::string getName() const                    { return m_name;}
    inline QVector3D getCOM() const                                { return m_COM; }

    inline bool hasSpherePack() const { return m_hasSpherePack;}
    inline bool isSkinned() const { return m_isSkinned; }

    //The class used for sphere packing. This is global because
    //there were some issues causes when it was private and I used
    //getters and setters
    std::shared_ptr<SpherePack> m_spherePack;

private:
    //Don't let a mesh get constructed without the neccesary data
    Mesh() = default;

    //Function used to check whether a vector is (0,0,0) but allowing for accuracy errors
    bool vectorIsSmall(const QVector3D &a);

    //Colour of the mesh
    QVector4D m_colour;

    //The name of the mesh
    std::string m_name;

    //Array and buffer object for the mesh
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

    //Array and buffer object for the point field
    QOpenGLVertexArrayObject m_vaoSkin;
    QOpenGLBuffer m_vboSkin;

    //Containers for the mesh data
    vector_V m_verts;
    vector_V m_norms;
    uint_V m_meshIndex;
    QVector3D m_COM;

    //Container for the positions of all the skinned vertices of the mesh
    vector_V m_skinnedVerts;

    //Container used to store the relative positional vectors of vertices to
    //sphere positions
    std::vector< std::vector< std::pair< uint, QVector3D> > > m_vertSkinVectors;

    //Wireframe state of the mesh
    bool m_wireframeMode;

    //Whether sphere packing has been done on this mesh or not
    bool m_hasSpherePack;

    //And finally if the mesh is skinned or not
    bool m_isSkinned;
};

#endif // MESH_H_
