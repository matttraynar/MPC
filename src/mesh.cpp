#include "mesh.h"

Mesh::Mesh(QVector4D colour, const std::string name)
{
    //Set the colour to the specified value
    m_colour = colour;
    m_name = name;
    m_hasSpherePack = false;
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

void Mesh::draw()
{
    //Bind to the vertex array object
    m_vao.bind();

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
    m_vao.release();
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


