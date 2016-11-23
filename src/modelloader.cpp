#include "modelloader.h"

#include <iostream>

ModelLoader::ModelLoader()
{

}

bool ModelLoader::loadFromFile(const char *filepath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath,
                                             aiProcess_GenSmoothNormals |
                                             aiProcess_CalcTangentSpace |
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType);

    if(!scene)
    {
        qWarning() << "Error loading file: ASSIMP: "<<importer.GetErrorString();
        return false;
    }

    if(scene->HasMeshes())
    {
        for(uint i = 0; i < scene->mNumMeshes; ++i)
        {
            m_meshes.push_back(processMesh(scene->mMeshes[i]));
        }
    }
    else
    {
        qWarning() <<"Error: There aren't any meshes in this scene";
        return false;
    }

    if(scene->mRootNode != NULL)
    {
        Node *root = new Node;
        processNode(scene, scene->mRootNode, 0, *root);
        m_rootNode = root;
    }
    else
    {
        qWarning() << "Error loading model, no root node";
        return false;
    }

    return true;
}

QSharedPointer<Mesh> ModelLoader::processMesh(aiMesh *mesh)
{
    QSharedPointer<Mesh> newMesh(new Mesh);
    newMesh->name = mesh->mName.length != 0 ? mesh->mName.C_Str() : "";
    newMesh->iOffset = m_indices.size();
    uint prevICount = m_indices.size();
    int vertIOffset = m_verts.size()/3;

    if(mesh->mNumVertices > 0)
    {
        for(uint i = 0; i < mesh->mNumVertices; ++i)
        {
            aiVector3D &vec = mesh->mVertices[i];

            m_verts.push_back(vec.x);
            m_verts.push_back(vec.y);
            m_verts.push_back(vec.z);
        }
    }

    if(mesh->HasNormals())
    {
        for(uint i = 0; i < mesh->mNumVertices; ++i)
        {
            aiVector3D &norm = mesh->mNormals[i];

            m_norms.push_back(norm.x);
            m_norms.push_back(norm.y);
            m_norms.push_back(norm.z);
        }
    }

    for(uint i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace* face = &mesh->mFaces[i];

        if(face->mNumIndices != 3)
        {
            qWarning() <<"Warning: Mesh has non-triangluar face, ignoring primitive";
            continue;
        }

        m_indices.push_back(face->mIndices[0] + vertIOffset);
        m_indices.push_back(face->mIndices[1] + vertIOffset);
        m_indices.push_back(face->mIndices[2] + vertIOffset);
    }

    newMesh->iCount = m_indices.size() - prevICount;

    return newMesh;
}

void ModelLoader::processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode)
{
    newNode.name = node->mName.length != 0 ? node->mName.C_Str() : "";

    newNode.trans = QMatrix4x4(node->mTransformation[0]);

    newNode.meshes.resize(node->mNumMeshes);

    for(uint i = 0; i < node->mNumMeshes; ++i)
    {
        QSharedPointer<Mesh> mesh = m_meshes[node->mMeshes[i]];
        newNode.meshes[i] = mesh;
    }

    for(uint i = 0; i < node->mNumChildren; ++i)
    {
        newNode.nodes.push_back(Node());
        processNode(scene, node->mChildren[i], parentNode, newNode.nodes[i]);
    }
}

void ModelLoader::getBufferData(QVector<float> **verts, QVector<float> **norms, QVector<uint> **indices)
{
    if(verts != 0)
    {
        *verts = &m_verts;
    }

    if(norms != 0)
    {
        *norms = &m_norms;
    }

    if(indices != 0)
    {
        *indices = &m_indices;
    }
}
