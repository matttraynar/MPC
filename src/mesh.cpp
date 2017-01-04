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

    m_oldSpherePositions = copy.m_oldSpherePositions;
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
        m_skinData.resize(numControlSpheres, std::make_pair(LARGE_PVE, LARGE_PVE));

        for(uint j = 0; j < m_spherePack->getSphereNum(); ++j)
        {
            float dist = (m_verts[i] - m_spherePack->getSphereAt(j)).length();

            if(dist == m_skinData.back().second)
            {
                continue;
            }
            else if(dist < m_skinData.back().second)
            {
                m_skinData.pop_back();

                m_skinData.push_back(std::make_pair(j, dist));

                std::sort(m_skinData.begin(), m_skinData.end(), SortPair());
            }
        }

        float total = 0.0f;

        for(uint j = 0; j < numControlSpheres; ++j)
        {
            total += m_skinData[j].second;
        }

        for(uint j = 0; j < numControlSpheres; ++j)
        {
            //Set the distance as a percentage of all the distances.
            //Also invert so small distance = large weight.
            m_skinData[j].second = (1.0f - (m_skinData[j].second / total))/(numControlSpheres - 1);
        }

        m_vertSkinData.push_back(m_skinData);
    }

    m_skinnedVerts = m_verts;
}

void Mesh::updateSkinnedMesh(const vector_V &spherePositions)
{
    vector_V sphereDisplacements;
//    m_skinnedVerts.clear();

    for(uint i = 0; i < m_spherePack->getSphereNum(); ++i)
    {
        sphereDisplacements.push_back(spherePositions[i] - m_spherePack->getSphereAt(i));
    }

//    vector_V updatedVerts;

    for(uint i = 0; i < m_verts.size(); i++)
    {
//        auto vectorPosition = std::find_if(updatedVerts.begin(), updatedVerts.end(), FindVector(m_verts[i]));

        QVector3D disp(0,0,0);

        for(uint j = 0; j < m_vertSkinData[i].size(); ++j)
        {
            disp += m_vertSkinData[i][j].second * sphereDisplacements[m_vertSkinData[i][j].first];
        }


        m_skinnedVerts[i] = m_verts[i] + disp;

//        if(vectorPosition == updatedVerts.end())
//        {
//            m_skinnedVerts[i] = m_verts[i] + disp;
////            m_skinnedVerts.push_back(m_verts[i] + disp);
//        }
//        else
//        {
//            int vectorIndex = vectorPosition - updatedVerts.begin();

//            m_skinnedVerts[i] = (m_skinnedVerts[vectorIndex] + (m_verts[i] + disp)) / 2.0f;
////            m_skinnedVerts.push_back(m_skinnedVerts[vectorIndex]);
//        }

//        updatedVerts.push_back(m_verts[i]);
    }
}
