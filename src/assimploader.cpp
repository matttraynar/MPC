#include "assimploader.h"

#include <iostream>

AssimpLoader::~AssimpLoader()
{

}

void AssimpLoader::loadMesh(const char *filepath)
{
    m_colour = QVector4D(0.0f, 0.0f, 0.0f, 0.0f);

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
                                             aiProcess_GenSmoothNormals |
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType);

    if(!scene)
    {
        qWarning() << "Error: File didn't load: ASSIMP: " << importer.GetErrorString();
        exit(1);
    }

    if(scene->HasMeshes())
    {
        for(uint i = 0; i < scene->mNumMeshes; ++i)
        {
            uint numFaces = scene->mMeshes[i]->mNumFaces;

            for(uint j = 0; j < numFaces; ++j)
            {
                auto face = scene->mMeshes[i]->mFaces[j];

                m_meshIndex.push_back(face.mIndices[0]);
                m_meshIndex.push_back(face.mIndices[1]);
                m_meshIndex.push_back(face.mIndices[2]);
            }

            uint numVerts = scene->mMeshes[i]->mNumVertices;

            for(uint j = 0; j < numVerts; ++j)
            {
                auto vert = scene->mMeshes[i]->mVertices[j];
                auto norm = scene->mMeshes[i]->mNormals[j];

                m_verts.push_back(QVector3D(vert.x,vert.y,vert.z));
                m_norms.push_back(QVector3D(norm.x,norm.y,norm.z));
            }
        }
    }

    m_colour = QVector4D(m_verts[0][0],m_verts[0][1],m_verts[0][2],1.0f);
}

void AssimpLoader::prepareMesh(QOpenGLShaderProgram& program)
{
    m_vao.create();
    m_vao.bind();

    m_vbo.create();
    m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo.bind();
    m_vbo.allocate(&m_verts[0], m_verts.size() * sizeof(GLfloat) * 3);

    program.enableAttributeArray("vertexPos");
    program.setAttributeArray("vertexPos", GL_FLOAT, 0, 3);

    m_vbo.release();

    m_nbo.create();
    m_nbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_nbo.bind();
    m_nbo.allocate(&m_norms[0], m_norms.size() * sizeof(GLfloat) * 3);

    program.enableAttributeArray("vertexNorm");
    program.setAttributeArray("vertexNorm", GL_FLOAT, 0, 3);

    m_nbo.release();

    m_ibo.create();
    m_ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_ibo.bind();
    m_ibo.allocate(&m_meshIndex[0], m_meshIndex.size() * sizeof(uint));

    program.setUniformValue("mCol",m_colour);

    m_vao.release();
}

void AssimpLoader::draw()
{
    m_vao.bind();

    glDrawElements(GL_TRIANGLES, m_meshIndex.size(), GL_UNSIGNED_INT, &m_meshIndex[0]);

    m_vao.release();
}
