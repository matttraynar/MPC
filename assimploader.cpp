#include "assimploader.h"

AssimpLoader::AssimpLoader()
{

}

void AssimpLoader::LoadMesh(const char *filepath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
                                             aiProcess_GenSmoothNormals |
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType);

    if(!scene)
    {
        qtWarning() << "Error: File didn't load: ASSIMP: " << importer.GetErrorString();
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
                m_meshIndex.push_back(QVector3D(face.mIndices[0] + iOffset, face.mIndices[1] + iOffset, face.mIndices[2] + iOffset));

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

void AssimpLoader::DrawMesh(int meshID)
{

}
