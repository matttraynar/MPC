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
        uint iOffset = 0;

        for(uint i = 0; i < scene->mNumMeshes; ++i)
        {
            uint numFaces = scene->mMeshes[i]->mNumFaces;

            for(uint j = 0; j < numFaces; ++j)
            {
                auto face = scene->mMeshes[i]->mFaces[j];
                m_meshIndex.push_back(face.mIndices[0]);
                m_meshIndex.push_back(face.mIndices[1]);
                m_meshIndex.push_back(face.mIndices[2]);

                iOffset += 3;
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

void AssimpLoader::print()
{
    for(int i = 0; i < m_verts.size(); ++i)
    {
        QVector3D newVector = m_verts[i];
        std::cout<<"New verts: "<<newVector.x()<<' '<<newVector.y()<<' '<<newVector.z()<<"\n--------------------\n";
    }
}
