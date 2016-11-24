#ifndef ASSIMPLOADER_H
#define ASSIMPLOADER_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QtGui>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QVector3D>

#include <vector>

struct Mesh
{
    uint numFaces;
    uint numVerts;

    std::vector<QVector3D> verts;
    std::vector<QVector3D> norms;
    std::vector<QVector3D> indices;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QOpenGLBuffer nbo;
    QOpenGLBuffer ibo;
};

class AssimpLoader
{
public:
    AssimpLoader();
    ~AssimpLoader();

    void loadMesh(const char* filepath);
    void prepareMesh();
    void setShaderProgram(QOpenGLShaderProgram program);
    void draw();

    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<uint> m_meshIndex;

private:
    QOpenGLShaderProgram m_pgm;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

    std::vector<Mesh> m_meshes;
};

#endif // ASSIMPLOADER_H
