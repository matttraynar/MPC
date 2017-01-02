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

class Mesh
{
public:
    //Ctors
    Mesh() = default;
    Mesh(QVector4D colour, const std::string name);

    //Dtor
    ~Mesh();

    //Methods for loading, passing to the shader and drawing the mesh
    void loadMesh(const char* filepath);
    void prepareMesh(QOpenGLShaderProgram &program);
    void draw();

    //Method which creates a sphere pack for the mesh
    void runSpherePackAlgorithm(float radius);

    //Set methods
    void setWireMode();
    void setColour(QVector4D colour);
    void setName(const std::string name) { m_name = name;}

    //Get methods
    inline std::vector<QVector3D> getVerts() const { return m_verts; }
    inline QVector4D getColour() const                  { return m_colour; }
    inline std::string getName() const                    { return m_name;}
    inline QVector3D getCOM() const                                { return m_COM; }

    std::shared_ptr<SpherePack> m_spherePack;

private:
    //Colour of the mesh
    QVector4D m_colour;

    std::string m_name;

    //Array and buffer object for the mesh
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

    //Array and buffer object for the point field
    QOpenGLVertexArrayObject m_vaoP;
    QOpenGLBuffer m_vboP;

    //Containers for the mesh data
    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<uint> m_meshIndex;
    QVector3D m_COM;

    //Wireframe state of the mesh
    bool m_wireframeMode;
};

#endif // MESH_H_
