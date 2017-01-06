#include "mesh.h"

Mesh::Mesh(QVector4D colour, const std::string name)
{
    //Set up the variables for the mesh
    m_colour = colour;
    m_name = name;
    m_hasSpherePack = false;
    m_isSkinned = false;
}

Mesh::Mesh(const Mesh &copy)
{
    //The copy constructor. Simply copy all the data across
    //to 'this'
    m_spherePack = copy.m_spherePack;

    m_colour = copy.m_colour;
    m_name = copy.m_name;

    m_verts = copy.m_verts;
    m_norms = copy.m_norms;
    m_meshIndex = copy.m_meshIndex;

    m_COM = copy.m_COM;

    m_skinnedVerts = copy.m_skinnedVerts;
    m_vertSkinVectors = copy.m_vertSkinVectors;

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
    //Check for the skinned VAO
    if(!m_vaoSkin.isCreated())
    {
        //Create it if it hasn't been already
        m_vaoSkin.create();
    }

    //Bind the vao to the current context
    m_vaoSkin.bind();

    //Create a buffer object with the skinned vertex data
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

    //With the verts changing because of the skinning operation
    //the face normals will also change. These are recalculated
    //using the new skinned vertex positions
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
    //as with the vertex buffer object. This uses the skinned norms
    //we just created
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

    //Finally set the state of 'skinned' for future use
    m_isSkinned = true;
}

void Mesh::draw()
{
    //Bind to the correct vertex array object depending on whether
    //we are using the skinned model or not
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

    //Finally release the appropriate vertex array object
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
    //Toggle the wireframe state of the mesh
    m_wireframeMode = !m_wireframeMode;
}

void Mesh::setColour(QVector4D colour)
{
    //Method for setting the colour of the mesh
    m_colour = colour;
}

void Mesh::runSpherePackAlgorithm(float radius)
{
    //Mark that the mesh has a sphere pack
    m_hasSpherePack = true;

    //Reset the sphere pack variable using the parameterised
    //constructor. This effectively runs the sphere pack
    m_spherePack.reset(new SpherePack(m_verts, m_meshIndex, radius));
    qInfo()<<"Sphere packing finished on"<<(QString)m_name.c_str();
}

void Mesh::skinMeshToSpheres(uint numControlSpheres)
{
    //First check that there are enough spheres for the desired
    //control number. If not just clamp the value
    if(numControlSpheres > m_spherePack->getSphereNum())
    {
        numControlSpheres = m_spherePack->getSphereNum();
    }

    //Iterate through every vertex of the mesh
    for(uint i = 0; i < m_verts.size(); ++i)
    {
        //Create two containers, one for storing distances
        //between the spheres and the current vert and a second
        //for storing the vector between the spheres and the
        //current vert. By storing the index of the sphere effecting
        //the data as a pair the sphere can be accessed by index
        //later
        std::vector< std::pair< uint, float> > skinData;
        std::vector< std::pair< uint, QVector3D> > skinVectors;

        //Set the maximum distance to a value which will
        //definitely be overwritten
        float maxDist = -1.0f;

        //Iterate through the sphere pack
        for(uint j = 0; j < m_spherePack->getSphereNum(); ++j)
        {
            //Get the distance between the sphere and the current vert
            float dist = (m_verts[i] - m_spherePack->getSphereAt(j)).length();

            //If the skin data is not yet at the size of the control spheres
            //the data is automatically added
            if(skinData.size() <= numControlSpheres)
            {
                skinData.push_back(std::make_pair(j, dist));

                std::sort(skinData.begin(), skinData.end(), SortPair());

                maxDist = skinData.back().second;
                continue;
            }

            //Otherwise we need to check the distance
            if(dist == maxDist)
            {
                //In this case there is already a sphere which has the
                //same distance, not much point in duplicating another one
                continue;
            }
            //The distance is less than the current max distance (a closer
            //sphere to the current vert has been found)
            else if(dist < maxDist)
            {
                //Because the container is sorted by distance smallest->largest
                //the last element can just be replaced with the new value

                skinData[skinData.size() - 1] = std::make_pair(j, dist);

                //The vector does need to be sorted though to make sure the
                //smallest->largest order is retained
                std::sort(skinData.begin(), skinData.end(), SortPair());

                //Set the maximum value again
                maxDist = skinData.back().second;
            }
        }

        //At this point the spheres have all been sorted and the
        //closest ones stored in skinData. The indices of these
        //spheres (stored in the first element of the pair) can be
        //used to set data in the other container
        for(uint j = 0; j <= numControlSpheres; ++j)
        {
            skinVectors.push_back(std::make_pair(skinData[j].first, m_verts[i] - m_spherePack->getSphereAt(skinData[j].first)));
        }

        //Finally the data is stored for later use in the skinning stage
        m_vertSkinVectors.push_back(skinVectors);
    }

    //The intial state of the skinned verts will just be their default position
    m_skinnedVerts = m_verts;

    //Calculate the centroid of the mesh using the sphere pack
    m_COM = QVector3D(0,0,0);

    for(uint i = 0; i < m_spherePack->getSphereNum(); ++i)
    {
        m_COM += m_spherePack->getSphereAt(i);
    }

    m_COM /= m_spherePack->getSphereNum();
}

void Mesh::updateSkinnedMesh(const vector_V &spherePositions)
{
    //Create a container to store rotation matrices in
    std::vector<QMatrix4x4> rotationMatrices;

    //Calculate the centroid of all of the spheres
    QVector3D com(0,0,0);

    for(uint i = 0; i < spherePositions.size(); ++i)
    {
        com += spherePositions[i];
    }

    com /= spherePositions.size();

    //Iterate through the sphere pack
    for(uint i = 0; i < m_spherePack->getSphereNum(); ++i)
    {
        //Get the original position of the sphere and the current
        //position. Do these relative to the center of the pack
        QVector3D ogPos = m_spherePack->getSphereAt(i) - m_COM;
        QVector3D newPos = spherePositions[i] - com;

        //Normalize both vectors
        ogPos.normalize();
        newPos.normalize();

        //Check if the difference between the two is actually
        //large enough for any rotation
        if(vectorIsSmall(ogPos - newPos))
        {
            //If not we can skip this, simply store the identity
            //matrix in the rotation container
            QMatrix4x4 rotation;
            rotation.setToIdentity();

            rotationMatrices.push_back(rotation);
            continue;
        }

        //Otherwise we need to calculate the rotation matrix
        //which maps the original position onto the new position
        //For more info take a look at:
        //math.stackexchange.com/questions/180418/calculate-rotation-matrix-to-align-vector-a-to-vector-b-in-3d/180436#180436
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

        //Finally add the matrix to the container
        rotationMatrices.push_back(rotation);
    }

    //Iterate through all the verts in the mesh
    for(uint i = 0; i < m_verts.size(); ++i)
    {
        //Set up to vectors for storing positional changes
        QVector3D translate(0,0,0);
        QVector3D rotate(0,0,0);

        //Go through the spheres that will adjust the point,
        //the indices of which we stored earlier
        for(uint j = 0; j < m_vertSkinVectors[i].size(); ++j)
        {
            //Get the vector which will rotate the point
            rotate += (rotationMatrices[m_vertSkinVectors[i][j].first] * m_vertSkinVectors[i][j].second);

            //And the vector to translate by
            translate += spherePositions[m_vertSkinVectors[i][j].first];
        }

        //Average the rotation/translation for all the spheres considered
        rotate /= m_vertSkinVectors[i].size();
        translate /= m_vertSkinVectors[i].size();

        //Finally set the skinned vertex position to the sum
        //of the translation and the rotation
        m_skinnedVerts[i] = translate + rotate;
    }
}

bool Mesh::vectorIsSmall(const QVector3D &a)
{
    //Function which checks whether the given vector is (pretty much) 0
    return ((a.x() >= -0.0001f && a.x() <= 0.0001f) &&
              (a.y() >= -0.0001f && a.y() <= 0.0001f) &&
              (a.z() >= -0.0001f && a.z() <= 0.0001f));
}
