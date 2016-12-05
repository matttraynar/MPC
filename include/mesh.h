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

class Mesh
{
public:
    //Ctors
    Mesh() = default;
    Mesh(QVector4D colour);

    //Dtor
    ~Mesh();

    //Methods for loading, passing to the shader and drawing the mesh
    void loadMesh(const char* filepath);
    void prepareMesh(QOpenGLShaderProgram &program);
    void draw();

    void generateDistanceField();

    //Method which creates a sphere pack for the mesh
    void packSpheres();

    //Setter methods
    void setWireMode();
    void setColour(QVector4D colour);

    //Get methods
    inline std::vector<QVector3D> getVerts() const { return m_verts; }
    inline QVector3D getSphereAt(int index) const  { return m_spherePositions[index];}
    inline int getSphereNum() const                { return m_spherePositions.size(); }

    //Colour of the mesh
    QVector4D m_colour;

private:
    void calculateMAABB(float &xMin, float &xMax,
                                    float &yMin, float &yMax,
                                    float &zMin, float &zMax);

    //Array and buffer object for the mesh
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

    //Containers for the mesh data
    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<uint> m_meshIndex;

    //Container with sphere positions
    std::vector<QVector3D> m_spherePositions;

    //Wireframe state
    bool m_wireframeMode;
};

#endif // MESH_H_
