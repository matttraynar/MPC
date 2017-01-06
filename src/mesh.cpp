#include "mesh.h"

Mesh::Mesh(QVector4D colour, const std::string name)
{
    //Set the colour to the specified value
    m_colour = colour;
    m_name = name;
    m_hasSpherePack = false;
    m_isSkinned = false;
}

Mesh::Mesh(const Mesh &copy)
{
    m_spherePack = copy.m_spherePack;

    m_colour = copy.m_colour;
    m_name = copy.m_name;

    m_verts = copy.m_verts;
    m_norms = copy.m_norms;
    m_meshIndex = copy.m_meshIndex;

    m_COM = copy.m_COM;

    m_skinnedVerts = copy.m_skinnedVerts;
    m_vertSkinData = copy.m_vertSkinData;
    m_skinData = copy.m_skinData;

    //Wireframe state of the mesh
    m_wireframeMode = copy.m_wireframeMode;
    m_hasSpherePack = copy.m_hasSpherePack;
    m_isSkinned = copy.m_isSkinned;
}

Mesh::~Mesh()
{

}

void Mesh::loadMesh(const char *filepath)
{
    //Set the wireframe mode
    m_wireframeMode = false;

    //Create a new assimp importer and scene with the file path
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
                                             aiProcess_GenSmoothNormals |
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType);

    //Check for loading errors
    if(!scene)
    {
        qWarning() << "Error: File didn't load: ASSIMP: " << importer.GetErrorString();
        exit(1);
    }

    //Check for meshes
    if(scene->HasMeshes())
    {
        //There are meshes so we can iterate through them
        for(uint i = 0; i < scene->mNumMeshes; ++i)
        {
            //Get the number of faces (just to make the next for loop more readable
            uint numFaces = scene->mMeshes[i]->mNumFaces;

            for(uint j = 0; j < numFaces; ++j)
            {
                //Get each face in the mesh and add the index to the index container
                auto face = scene->mMeshes[i]->mFaces[j];

                m_meshIndex.push_back(face.mIndices[0]);
                m_meshIndex.push_back(face.mIndices[1]);
                m_meshIndex.push_back(face.mIndices[2]);
            }

            //Update the num verts. This is again to make the next for loop readable
            uint numVerts = scene->mMeshes[i]->mNumVertices;

            for(uint j = 0; j < numVerts; ++j)
            {
                //Iterate throught the vertex and normal data in the current mesh
                auto vert = scene->mMeshes[i]->mVertices[j];
                auto norm = scene->mMeshes[i]->mNormals[j];

                //Add this data to the correct container
                m_verts.push_back(QVector3D(vert.x,vert.y,vert.z));
                m_norms.push_back(QVector3D(norm.x,norm.y,norm.z));

                m_COM += m_verts[m_verts.size() - 1];
            }

            m_COM /= numVerts;
        }
    }
}

void Mesh::prepareMesh(QOpenGLShaderProgram& program)
{
    //Create a vertex array object and bind it to the context
    m_vao.create();
    m_vao.bind();

    //Create a buffer object with the vertex data
    m_vbo.create();
    m_vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vbo.bind();
    m_vbo.allocate(&m_verts[0], (int)m_verts.size() * sizeof(GLfloat) * 3);

    //Tell the shader program the currently bound buffer object contains
    //vertex position data
    program.enableAttributeArray("vertexPos");
    program.setAttributeArray("vertexPos", GL_FLOAT, 0, 3);

    //Release the vertex buffer object
    m_vbo.release();

    //Next we create the normal buffer object, process is the same
    //as with the vertex buffer object
    m_nbo.create();
    m_nbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_nbo.bind();
    m_nbo.allocate(&m_norms[0], (int)m_norms.size() * sizeof(GLfloat) * 3);

    program.enableAttributeArray("vertexNorm");
    program.setAttributeArray("vertexNorm", GL_FLOAT, 0, 3);

    m_nbo.release();

    //Finally we have an index buffer object for storing the indices used
    //to draw the mesh with
    m_ibo.create();
    m_ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_ibo.bind();
    m_ibo.allocate(&m_meshIndex[0], (int)m_meshIndex.size() * sizeof(uint));

    //Don't release the ibo from the vao but do release the vertex array object,
    //we'll bind to it later when it is needed for drawing
    m_vao.release();
}

void Mesh::prepareSkinnedMesh(QOpenGLShaderProgram &program)
{
    //Create a vertex array object and bind it to the context
    if(!m_vaoSkin.isCreated())
    {
        m_vaoSkin.create();
    }
    else
    {
        m_vaoSkin.destroy();
        m_vaoSkin.create();
    }
    m_vaoSkin.bind();

    //Create a buffer object with the vertex data
    m_vboSkin.create();
    m_vboSkin.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vboSkin.bind();
    m_vboSkin.allocate(&m_skinnedVerts[0], (int)m_skinnedVerts.size() * sizeof(GLfloat) * 3);

    //Tell the shader program the currently bound buffer object contains
    //vertex position data
    program.enableAttributeArray("vertexPos");
    program.setAttributeArray("vertexPos", GL_FLOAT, 0, 3);

    //Release the vertex buffer object
    m_vboSkin.release();

    vector_V skinnedNorms;
    skinnedNorms.resize(m_skinnedVerts.size(), QVector3D(0,0,0));
    for(uint i = 0; i < m_meshIndex.size(); i += 3)
    {
        Triangle face;
        face.A = m_skinnedVerts[m_meshIndex[i]];
        face.B = m_skinnedVerts[m_meshIndex[i + 1]];
        face.C = m_skinnedVerts[m_meshIndex[i + 2]];

        QVector3D norm = face.getNormal();
        skinnedNorms[m_meshIndex[i]] = norm;
        skinnedNorms[m_meshIndex[i + 1]] = norm;
        skinnedNorms[m_meshIndex[i + 2]] = norm;
    }

    //Next we create the normal buffer object, process is the same
    //as with the vertex buffer object
    m_nbo.create();
    m_nbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_nbo.bind();
    m_nbo.allocate(&skinnedNorms[0], (int)skinnedNorms.size() * sizeof(GLfloat) * 3);

    program.enableAttributeArray("vertexNorm");
    program.setAttributeArray("vertexNorm", GL_FLOAT, 0, 3);

    m_nbo.release();

    //Finally we have an index buffer object for storing the indices used
    //to draw the mesh with
    m_ibo.create();
    m_ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_ibo.bind();
    m_ibo.allocate(&m_meshIndex[0], (int)m_meshIndex.size() * sizeof(uint));

    //Don't release the ibo from the vao but do release the vertex array object,
    //we'll bind to it later when it is needed for drawing
    m_vaoSkin.release();

    m_isSkinned = true;
}

void Mesh::draw()
{
    //Bind to the vertex array object
    if(m_isSkinned)
    {
        m_vaoSkin.bind();
    }
    else
    {
        m_vao.bind();
    }

    //Check if we need to draw in wireframe mode or not
    if(m_wireframeMode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    //Draw the mesh data using indexed elements
    glDrawElements(GL_TRIANGLES,  (int)m_meshIndex.size(), GL_UNSIGNED_INT, &m_meshIndex[0]);

    //Reset the polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //Finally release the vertex array object
    if(m_isSkinned)
    {
        m_vaoSkin.release();
    }
    else
    {
        m_vao.release();
    }
}

void Mesh::setWireMode()
{
    //Swap the wireframe mode accordingly
    m_wireframeMode ? m_wireframeMode = false : m_wireframeMode = true;
}

void Mesh::setColour(QVector4D colour)
{
    //Method for setting the colour of the mesh
    m_colour = colour;
}

void Mesh::runSpherePackAlgorithm(float radius)
{
    m_hasSpherePack = true;
    m_spherePack.reset(new SpherePack(m_verts, m_meshIndex, radius));
}

void Mesh::skinMeshToSpheres(uint numControlSpheres)
{
    if(numControlSpheres > m_spherePack->getSphereNum())
    {
        numControlSpheres = m_spherePack->getSphereNum();
    }

    for(uint i = 0; i < m_verts.size(); ++i)
    {
        m_skinData.clear();

        float maxDist = -1.0f;
        float totalDist = 0.0f;

        for(uint j = 0; j < m_spherePack->getSphereNum(); ++j)
        {
            float dist = (m_verts[i] - m_spherePack->getSphereAt(j)).length();

            if(m_skinData.size() <= numControlSpheres)
            {
                totalDist += dist;
                m_skinData.push_back(std::make_pair(j, dist));

                std::sort(m_skinData.begin(), m_skinData.end(), SortPair());

                maxDist = m_skinData.back().second;
                continue;
            }

            if(dist == maxDist)
            {
                continue;
            }
            else if(dist < maxDist)
            {
                totalDist -= m_skinData.back().second;
                m_skinData.pop_back();

                totalDist += dist;
                m_skinData.push_back(std::make_pair(j, dist));

                std::sort(m_skinData.begin(), m_skinData.end(), SortPair());

                maxDist = m_skinData.back().second;
            }
        }

        m_vertSkinVectors.push_back(std::make_pair(m_skinData[0].first, m_verts[i] - m_spherePack->getSphereAt(m_skinData[0].first)));

        for(uint j = 0; j <= numControlSpheres; ++j)
        {
            //Set the distance as a percentage of all the distances.
            //Also invert so small distance = large weight. Finally average
            //for each control sphere
            m_skinData[j].second = (1.0f - (m_skinData[j].second / totalDist)) / numControlSpheres;
        }

        m_vertSkinData.push_back(m_skinData);
    }

    m_skinnedVerts = m_verts;

    m_COM = QVector3D(0,0,0);
    for(uint i = 0; i < m_spherePack->getSphereNum(); ++i)
    {
        m_COM += m_spherePack->getSphereAt(i);
    }

    m_COM /= m_spherePack->getSphereNum();
}

void Mesh::updateSkinnedMesh(const vector_V &spherePositions)
{
    vector_V sphereAngleDisplacement;
    std::vector<QMatrix4x4> rotationMatrices;
    vector_V sphereDisplacements;
    std::vector<std::vector<QVector2D>> cosAndSine;

    QVector3D com(0,0,0);
    for(uint i = 0; i < spherePositions.size(); ++i)
    {
        com += spherePositions[i];
    }

    com /= spherePositions.size();

//    float Xangle = 0.0f;
//    float Yangle = 0.0f;
//    float Zangle = 0.0f;
    int count = 0;

    for(uint i = 0; i < m_spherePack->getSphereNum(); ++i)
    {
        sphereDisplacements.push_back(spherePositions[i] - m_spherePack->getSphereAt(i));

        QVector3D ogPos = m_spherePack->getSphereAt(i) - m_COM;
        QVector3D newPos = spherePositions[i] - com;

        ogPos.normalize();
        newPos.normalize();

        float dot = QVector3D::dotProduct(ogPos, newPos);
        QVector3D cross = QVector3D::crossProduct(ogPos, newPos);
        float crossLength = cross.length();

        QMatrix4x4 G(             dot, -crossLength, 0.0f, 0.0f,
                             crossLength,               dot, 0.0f, 0.0f,
                                         0.0f,               0.0f, 1.0f, 0.0f,
                                         0.0f,               0.0f, 0.0f, 1.0f);

        QVector3D c2 = newPos - (dot * ogPos);
        c2.normalize();

        QMatrix4x4 F;
        F.setColumn(0, QVector4D(ogPos, 0.0f));
        F.setColumn(1, QVector4D(c2, 0.0f));
        F.setColumn(2, QVector4D(QVector3D::crossProduct(newPos, ogPos), 0.0f));
        F.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

        QMatrix4x4 rotation = F * G * F.inverted();
        rotationMatrices.push_back(rotation);

        std::vector<QVector2D> tmpStore;

        float cos = QVector3D::dotProduct(QVector3D(ogPos.x(), 0.0f, ogPos.z()).normalized(), QVector3D(newPos.x(), 0.0f, newPos.z()).normalized());
        float sin = QVector3D::crossProduct(QVector3D(ogPos.x(), 0.0f, ogPos.z()).normalized(), QVector3D(newPos.x(), 0.0f, newPos.z()).normalized()).length();
        QVector3D angles;

        angles[0] = acos(cos);

//        if(i == 100)
//        {
//            Yangle = acos(cos);
//            count++;
//        }

        tmpStore.push_back(QVector2D(cos, sin));

        cos = QVector3D::dotProduct(QVector3D(0.0f, ogPos.y(), ogPos.z()).normalized(), QVector3D(0.0f, newPos.y(), newPos.z()).normalized());
        sin = QVector3D::crossProduct(QVector3D(0.0f, newPos.y(), newPos.z()).normalized(), QVector3D(0.0f, ogPos.y(), ogPos.z()).normalized()).length();

        angles[1] = acos(cos);
//        if(i == 100)
//        {
//            Xangle = acos(cos);
//        }
        tmpStore.push_back(QVector2D(cos, sin));

        cos = QVector3D::dotProduct(QVector3D(ogPos.x(), ogPos.y(), 0.0f).normalized(), QVector3D(newPos.x(), newPos.y(), 0.0f).normalized());
        sin = QVector3D::crossProduct(QVector3D(ogPos.x(), ogPos.y(), 0.0f).normalized(), QVector3D(newPos.x(), newPos.y(), 0.0f).normalized()).length();
        angles[2] = acos(cos);
//        if(i == 100)
//        {
//            Zangle = acos(cos);
//        }
        tmpStore.push_back(QVector2D(cos, sin));

        cosAndSine.push_back(tmpStore);

        sphereAngleDisplacement.push_back(angles);

    }

//    Xangle /= count;
//    Yangle /= count;
//    Zangle /= count;

    for(uint i = 0; i < m_verts.size(); ++i)
    {
        QVector3D move(0,0,0);
        QVector3D disp(0,0,0);

        for(uint j = 0; j < 1/*m_vertSkinData[i].size()*/; ++j)
        {
            QVector3D rotatedPoint = disp;
            float Yangle = sphereAngleDisplacement[m_vertSkinData[i][j].first].x();
            float Xangle = sphereAngleDisplacement[m_vertSkinData[i][j].first].y();
            float Zangle = sphereAngleDisplacement[m_vertSkinData[i][j].first].z();

            rotatedPoint[1] += ((m_verts[i].y() * cos(Xangle)) -
                                      (m_verts[i].z() * sin(Xangle)));

            rotatedPoint[2] += ((m_verts[i].y() * sin(Xangle)) +
                                      (m_verts[i].z() * cos(Xangle)));

            rotatedPoint[0] += ((m_verts[i].x() * cos(Yangle)) +
                                        (m_verts[i].z() * sin(Yangle)));

            rotatedPoint[2] += ((m_verts[i].z() * cos(Yangle)) -
                                        (m_verts[i].x() * sin(Yangle)));

            rotatedPoint[0] += ((m_verts[i].x() * cos(Zangle)) -
                                        (m_verts[i].y() * sin(Zangle)));

            rotatedPoint[1] += ((m_verts[i].x() * sin(Zangle)) +
                                        (m_verts[i].y() * cos(Zangle)));

//            disp += rotatedPoint;
//            disp /= 2.0f;

            QMatrix4x4 rotation((cos(Zangle) * cos(Yangle) * cos(Xangle)) - (sin(Zangle) * sin(Xangle)), -(cos(Zangle) * cos(Yangle) * cos(Xangle)) - (sin(Zangle) * sin(Xangle)), cos(Zangle) * sin(Yangle), 0.0f,
                                          (sin(Zangle) * cos(Yangle) * cos(Xangle)) + (cos(Zangle) * sin(Xangle)), -(sin(Zangle) * cos(Yangle) * sin(Xangle)) + (cos(Zangle) * cos(Xangle)), sin(Zangle) * sin(Yangle), 0.0f,
                                          -sin(Yangle) * cos(Xangle), sin(Yangle) * sin(Xangle), cos(Yangle), 0.0f,
                                            0.0f, 0.0f, 0.0f, 1.0f);

//            disp += (rotationMatrices[m_vertSkinData[i][j].first] * (m_verts[i] - m_COM)) + com;  //* m_vertSkinData[i][j].second;
//            disp /= 2.0f;

            qInfo()<<"Size: "<<m_vertSkinVectors.size();
            disp = (rotationMatrices[m_vertSkinData[i][0].first] * m_vertSkinVectors[i].second);
//            disp += rotatedPoint * m_vertSkinData[i][j].second;

//            disp[0] += ((m_verts[i].x() * cosAndSine[m_vertSkinData[i][j].first][0].x()) -
//                             (m_verts[i].z() * cosAndSine[m_vertSkinData[i][j].first][0].y()));

//            disp[2] += ((m_verts[i].x() * cosAndSine[m_vertSkinData[i][j].first][0].y()) +
//                             (m_verts[i].z() * cosAndSine[m_vertSkinData[i][j].first][0].x()));

//            disp[1] += ((m_verts[i].y() * cosAndSine[m_vertSkinData[i][j].first][1].x()) -
//                             (m_verts[i].z() * cosAndSine[m_vertSkinData[i][j].first][1].y()));

//            disp[2] += ((m_verts[i].y() * cosAndSine[m_vertSkinData[i][j].first][1].y()) +
//                             (m_verts[i].z() * cosAndSine[m_vertSkinData[i][j].first][1].x()));

//            disp[0] += ((m_verts[i].x() * cosAndSine[m_vertSkinData[i][j].first][2].x()) -
//                             (m_verts[i].y() * cosAndSine[m_vertSkinData[i][j].first][2].y()));

//            disp[1] += ((m_verts[i].x() * cosAndSine[m_vertSkinData[i][j].first][2].y()) +
//                             (m_verts[i].y() * cosAndSine[m_vertSkinData[i][j].first][2].x()));

//            disp /= 2.0f;


//            disp *= m_vertSkinData[i][j].second;

//            move += m_vertSkinData[i][j].second * sphereDisplacements[m_vertSkinData[i][j].first];

        }
//        disp += m_COM;

//        disp /= m_vertSkinData[i].size();

        m_skinnedVerts[i] = spherePositions[m_vertSkinVectors[i].first] + disp;
//        m_skinnedVerts[i] = (m_verts[i] + move + disp);//+ com);// / m_vertSkinData[i].size();
//        m_skinnedVerts[i][0] /= m_vertSkinData[i].size();
//        m_skinnedVerts[i][1] /= m_vertSkinData[i].size();
//        m_skinnedVerts[i][2] /= m_vertSkinData[i].size();
    }
//    vector_V sphereAngleDisplacement;
//    std::vector<float> distances;
//    vector_V sphereDisplacements;

//    QVector3D com(0,0,0);
//    for(uint i = 0; i < spherePositions.size(); ++i)
//    {
//        com += spherePositions[i];
//    }

//    com /= spherePositions.size();


//    for(uint i = 0; i < m_spherePack->getSphereNum(); ++i)
//    {
//        /*QVector3D ogPos = m_spherePack->getSphereAt(i);
//        QVector3D newPos = spherePositions[i];
//        QVector3D angles(0,0,0);

////        angles.setX(getVectorAngle(QVector3D(ogPos.x(), ogPos.y(), 0.0f), QVector3D(newPos.x(), newPos.y(), 0.0f)));
////        angles.setY(getVectorAngle(QVector3D(0.0f, ogPos.y(), ogPos.z()), QVector3D(0.0f, newPos.y(), newPos.z())));
//        angles.setZ(getVectorAngle(QVector3D(ogPos.x(), 0.0f, ogPos.z()), QVector3D(newPos.x(), 0.0f, newPos.z())));

//        qInfo()<<"Angles: "<<angles;

//        sphereAngleDisplacement.push_back(angles);
//        distances.push_back(newPos.length()/ogPos.length());*/
//        sphereDisplacements.push_back(spherePositions[i] - m_spherePack->getSphereAt(i));
//    }

//    for(uint i = 0; i < m_verts.size(); ++i)
//    {
//        QVector3D disp(0,0,0);

//        for(uint j = 0; j < m_vertSkinData[i].size(); ++j)
//        {/*
////            QVector3D rotatedPoint = disp;

////            rotatedPoint[1] = ((m_verts[i].y() * cos(sphereAngleDisplacement[m_vertSkinData[i][j].first].x())) -
////                                      (m_verts[i].z() * sin(sphereAngleDisplacement[m_vertSkinData[i][j].first].x())));

////            rotatedPoint[2] = ((m_verts[i].y() * sin(sphereAngleDisplacement[m_vertSkinData[i][j].first].x())) +
////                                      (m_verts[i].z() * cos(sphereAngleDisplacement[m_vertSkinData[i][j].first].x())));

////            rotatedPoint[0] += ((m_verts[i].x() * cos(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())) +
////                                        (m_verts[i].z() * sin(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())));

////            rotatedPoint[2] += ((m_verts[i].z() * cos(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())) -
////                                        (m_verts[i].x() * sin(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())));

////            rotatedPoint[0] += ((m_verts[i].x() * cos(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())) -
////                                        (m_verts[i].y() * sin(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())));

////            rotatedPoint[1] += ((m_verts[i].x() * sin(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())) +
////                                        (m_verts[i].y() * cos(sphereAngleDisplacement[m_vertSkinData[i][j].first].z())));

////            disp += rotatedPoint * m_vertSkinData[i][j].second;*/
//            disp += m_vertSkinData[i][j].second * sphereDisplacements[m_vertSkinData[i][j].first];// + rotatedPoint);
//        }

////        disp /= m_vertSkinData[i].size();

//        m_skinnedVerts[i] = m_verts[i] + disp;

//    }
//    vector_V sphereDisplacements;

//    for(uint i = 0; i < m_spherePack->getSphereNum(); ++i)
//    {
//        sphereDisplacements.push_back(spherePositions[i] - m_spherePack->getSphereAt(i));
//    }

//    for(uint i = 0; i < m_verts.size(); i++)
//    {
//        QVector3D disp(0,0,0);

//        for(uint j = 0; j < m_vertSkinData[i].size(); ++j)
//        {
//            disp += m_vertSkinData[i][j].second * sphereDisplacements[m_vertSkinData[i][j].first];
//        }

//        m_skinnedVerts[i] = m_verts[i] + disp;
//    }
}

float Mesh::getVectorAngle(const QVector3D &a, const QVector3D &b)
{
    float dotProduct = QVector3D::dotProduct(a, b);
    float lengthA = a.length();
    float lengthB = b.length();

    return acos(dotProduct/(lengthA * lengthB));
}
