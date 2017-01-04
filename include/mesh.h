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

    //Method which creates a sphere pack for the mesh
    void runSpherePackAlgorithm(float radius);

    void skinMeshToSpheres(uint numControlSpheres);
    void updateSkinnedMesh(const vector_V &spherePositions);

    //Set methods
    void setWireMode();
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

    std::shared_ptr<SpherePack> m_spherePack;

private:
    Mesh() = default;

    inline bool sortPair(std::pair<uint, float> i, std::pair<uint, float> j) { return (i.second < j.second); }

    //Colour of the mesh
    QVector4D m_colour;

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

    vector_V m_oldSpherePositions;
    vector_V m_skinnedVerts;
    std::vector< std::vector< std::pair< uint, float> > > m_vertSkinData;
    std::vector< std::pair< uint, float> > m_skinData;

//    std::unordered_map< uint, uint_V> m_skinIndices;
//    std::unordered_map< uint, std::vector<float> > m_skinDistances;

    //Wireframe state of the mesh
    bool m_wireframeMode;
    bool m_hasSpherePack;
    bool m_isSkinned;
};

#endif // MESH_H_
