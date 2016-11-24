#include "assimploader.h"

#include <iostream>

AssimpLoader::AssimpLoader()
{

}

AssimpLoader::~AssimpLoader()
{

}

void AssimpLoader::loadMesh(const char *filepath)
{
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
            Mesh newMesh;
            newMesh.numFaces = scene->mMeshes[i]->mNumFaces;
//            uint numFaces = scene->mMeshes[i]->mNumFaces;

//            for(uint j = 0; j < numFaces; ++j)
            for(uint j = 0; j < newMesh.numFaces; ++j)
            {
                auto face = scene->mMeshes[i]->mFaces[j];
                newMesh.indices.push_back(face.mIndices[0]);
                newMesh.indices.push_back(face.mIndices[1]);
                newMesh.indices.push_back(face.mIndices[2]);
//                m_meshIndex.push_back(face.mIndices[0]);
//                m_meshIndex.push_back(face.mIndices[1]);
//                m_meshIndex.push_back(face.mIndices[2]);
            }

            newMesh.numVerts = scene->mMeshes[i]->mNumVertices;
//            uint numVerts = scene->mMeshes[i]->mNumVertices;

//            for(uint j = 0; j < numVerts; ++j)
            for(uint j = 0; j < newMesh.numVerts; ++j)
            {
                auto vert = scene->mMeshes[i]->mVertices[j];
                auto norm = scene->mMeshes[i]->mNormals[j];

                newMesh.verts.push_back(QVector3D(vert.x,vert.y,vert.z));
                newMesh.norms.push_back(QVector3D(norm.x,norm.y,norm.z));
//                m_verts.push_back(QVector3D(vert.x,vert.y,vert.z));
//                m_norms.push_back(QVector3D(norm.x,norm.y,norm.z));
            }

            m_meshes.push_back(newMesh);
        }
    }
}

void AssimpLoader::prepareMesh()
{
    m_pgm.bind();

    for(uint i = 0; i < m_meshes.size(); ++i)
    {
        m_meshes[i].vao.create();
    }
}
