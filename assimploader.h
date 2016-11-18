#ifndef ASSIMPLOADER_H
#define ASSIMPLOADER_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QOpenGLBuffer>
#include <QVector3D>

#include <vector>

class AssimpLoader
{
public:
    AssimpLoader();

    void LoadMesh(const char* filepath);
    void DrawMesh(int meshID);

private:
    std::vector<QOpenGLBuffer> m_meshVBOs;
    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<QVector3D> m_meshIndex;


};

#endif // ASSIMPLOADER_H
