#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <QVector4D>
#include <QVector3D>
#include <QString>
#include <QMatrix4x4>
#include <QVector>

struct MaterialInfo
{
    QString name;
    QVector3D amb;
    QVector3D dif;
    QVector3D spec;
    float shine;
};

struct LightInfo
{
    QVector4D pos;
    QVector3D strength;
};

struct Mesh
{
    QString name;
    int iCount;
    int iOffset;
    QSharedPointer<MaterialInfo> material;
};

struct Node
{
    QString name;
    QMatrix4x4 trans;
    QVector<QSharedPointer<Mesh>> meshes;
    QVector<Node> nodes;
};

class ModelLoader
{
public:
    ModelLoader();

    bool loadFromFile(const char * filepath);

    void getBufferData(QVector<float> **verts,
                       QVector<float> **norms,
                       QVector<uint> **indices);

    inline const Node * getNodeData() { return m_rootNode; }

    inline int getSize() const { return m_meshes.size(); }

private:
    QSharedPointer<Mesh> processMesh(aiMesh *mesh);

    void processNode(const aiScene *scene,
                     aiNode *node,
                     Node *parentNode,
                     Node &newNode);

    void transformToUnitCoords();

    void findObjDimensions(Node* node,
                           QMatrix4x4 trans,
                           QVector3D &minDimension,
                           QVector3D &maxDimension);

    QVector<float> m_verts;
    QVector<float> m_norms;
    QVector<uint>  m_indices;

    QVector<QSharedPointer<MaterialInfo>> m_materials;
    QVector<QSharedPointer<Mesh>> m_meshes;
    Node* m_rootNode;


};

#endif // MODELLOADER_H
