#ifndef ASSIMPLOADER_H
#define ASSIMPLOADER_H

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QtGui>
#include <QOpenGLBuffer>
#include <QVector3D>

#include <vector>

class AssimpLoader
{
public:
    AssimpLoader();
    ~AssimpLoader();

    void loadMesh(const char* filepath);
    void print();

    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<uint> m_meshIndex;

private:


};

#endif // ASSIMPLOADER_H
