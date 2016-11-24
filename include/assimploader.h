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

class AssimpLoader
{
public:
    AssimpLoader() = default;
    ~AssimpLoader();

    void loadMesh(const char* filepath);
    void prepareMesh(QOpenGLShaderProgram &program);
    void draw();

    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<uint> m_meshIndex;

private:
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

    QVector4D m_colour;
};

#endif // ASSIMPLOADER_H
