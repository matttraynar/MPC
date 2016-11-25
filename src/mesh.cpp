#include "mesh.h"

#include <iostream>

Mesh::Mesh(QVector4D colour)
{
    m_colour = colour;
}

Mesh::~Mesh()
{

}

void Mesh::loadMesh(const char *filepath)
{
    m_wireframeMode = false;

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

}

void Mesh::prepareMesh(QOpenGLShaderProgram& program)
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

    m_vao.release();
}

void Mesh::draw()
{
    m_vao.bind();

    if(m_wireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }


    glDrawElements(GL_TRIANGLES, m_meshIndex.size(), GL_UNSIGNED_INT, &m_meshIndex[0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    m_vao.release();
}

void Mesh::setWireMode()
{
    if(m_wireframeMode)
    {
        m_wireframeMode = false;
    }
    else
    {
        m_wireframeMode = true;
    }
}

void Mesh::setColour(QVector4D colour)
{
    m_colour = colour;
}
