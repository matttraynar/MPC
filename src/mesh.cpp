#include "mesh.h"

#include <iostream>

#define LARGE_PVE 1000000000

Mesh::Mesh(QVector4D colour)
{
    //Set the colour to the specified value
    m_colour = colour;

    m_radius = 1.0;
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
            }
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
    m_vbo.allocate(&m_verts[0], m_verts.size() * sizeof(GLfloat) * 3);

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
    m_nbo.allocate(&m_norms[0], m_norms.size() * sizeof(GLfloat) * 3);

    program.enableAttributeArray("vertexNorm");
    program.setAttributeArray("vertexNorm", GL_FLOAT, 0, 3);

    m_nbo.release();

    //Finally we have an index buffer object for storing the indices used
    //to draw the mesh with
    m_ibo.create();
    m_ibo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_ibo.bind();
    m_ibo.allocate(&m_meshIndex[0], m_meshIndex.size() * sizeof(uint));

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
    glDrawElements(GL_TRIANGLES, m_meshIndex.size(), GL_UNSIGNED_INT, &m_meshIndex[0]);

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

void Mesh::generateDistanceField()
{
    //Intialise the bounding box minimum and maximum values
    //to large numbers to ensure no errors when calculating the
    //Margined Axis Aligned Bouding Box (MAABB)
    float xMin = LARGE_PVE;
    float xMax = -LARGE_PVE;

    float yMin = LARGE_PVE;
    float yMax = -LARGE_PVE;

    float zMin = LARGE_PVE;
    float zMax = -LARGE_PVE;

    //Calculate the MAABB
    calculateMAABB(xMin, xMax, yMin, yMax, zMin, zMax);

    //Iterate through every unit point in the MAABB, essentially creating a 3D grid
    for(float y = yMin; y <= yMax; ++y)
    {
        std::vector< std::vector< int> > zDistances;
        for(float z = zMin; z <= zMax; ++z)
        {
            std::vector<int> xDistances;
            for(float x = xMin; x <= xMax; ++x)
            {
                //Create a new point and a container to store intersections in
                QVector2D point(y,z);
                std::vector<Intersections> pointHits;

                //Initialise the intersection count for this point to 0
                int intersectionCount = 0;

                //Look for triangles which have a 2D intersection (so might have a 3D one)
                getPotentialTriangles(point, pointHits);

                //There was at least one intersection, need to check 3D intersection
                if(pointHits.size() > 0)
                {
                    //Make a container for storing dot products later
                    std::vector<float> vertsDotContainer;
                    std::vector<float> edgeDotContainer;

                    //Iterate throught the intersections
                    for(uint i = 0; i < pointHits.size(); ++i)
                    {
                        //Store the data locally for ease of use
                        Intersections currentInter = pointHits[i];
                        Triangle currentTriangle = currentInter.tri;

                        //Calculate the normal of the current triangle
                        QVector3D triNorm = calculateTriNorm(currentTriangle);

                        //Store the barycentric coordinates locally too
                        float u = currentInter.baryCentric.x();
                        float v = currentInter.baryCentric.y();
                        float w = currentInter.baryCentric.z();

                        //If the intersection was on a vertex and the vertex x value is greater
                        //(i.e. along the (1,0,0) ray from the current grid position) store the
                        //dot product of the triangle norm and the ray direction vector. As the
                        //ray direction vector is (1,0,0) this equates to just the x component
                        if((currentInter.type = Vertex) &&
                                ((currentTriangle.C.x() >= x) ||
                                 (currentTriangle.B.x() >= x) ||
                                 (currentTriangle.A.x() >= x)))
                        {
                            vertsDotContainer.push_back(triNorm.x());
                        }

                        //Do the same with edges but store the dot product in a different container
                        else if((currentInter.type == Edge) &&
                                ((currentTriangle.C.x() >= x) ||
                                 (currentTriangle.B.x() >= x) ||
                                 (currentTriangle.A.x() >= x)))
                        {
                            edgeDotContainer.push_back(triNorm.x());
                        }
                        //Easy, the intersection was with a face, mark an intersection
                        else if((currentInter.type == Face) &&
                                (triNorm.x() >= x))
                        {
                            intersectionCount++;
                        }
                    }

                    //Next we need to test the intersections which were with vertices or
                    //edges to make sure we don't double count them. This will be important
                    //later. Basically if the sign of every value in the container is the same
                    //as the direction through the mesh:
                    //
                    //              Positive dot  = leaving the mesh
                    //              Negative dot = entering the mesh
                    //
                    //However in some cases (e.g. on an edge or vertex) multiple values
                    //can be recorded, by checking the signs are all the same errors like
                    //this can be avoided

                    bool allSame = true;
                    int lastSign = 0;

                    //First check if there were any vert intersections
                    if(vertsDotContainer.size() > 0)
                    {
                        //Check the sign of the first vertex dot product
                        ((vertsDotContainer[0] < 0) ? lastSign = -1 : lastSign = 1);

                        //Iterate through the container checking signs
                        for(uint i = 0; i < vertsDotContainer.size(); ++i)
                        {
                            if(((vertsDotContainer[i] < 0) && (lastSign > 0)) ||
                                    ((vertsDotContainer[i] > 0) && (lastSign < 0)))
                            {
                                //A value with a different sign was detected, mark this and
                                //break immediately (no need to continue looking)
                                allSame = false;
                                break;
                            }
                        }

                        //There was a correct intersection so increment the count
                        if(allSame)
                        {
                            intersectionCount++;
                        }
                    }

                    //Reset the boolean value for the next container
                    allSame = true;

                    //Now we do exactly the same, first checking if
                    //there were edge intersections
                    if(edgeDotContainer.size() > 0)
                    {
                        ((edgeDotContainer[0] < 0) ? lastSign = -1 : lastSign = 1);

                        for(uint i = 1; i < edgeDotContainer.size(); ++i)
                        {
                            if(((edgeDotContainer[i] < 0) && (lastSign > 0)) ||
                                    ((edgeDotContainer[i] > 0) && (lastSign < 0)))
                            {
                                allSame = false;
                                break;
                            }
                        }
                        if(allSame)
                        {
                            intersectionCount++;
                        }
                    }
                }

                //After all that we can check the intersection count.
                //An even number means the point is outside the mesh,
                //this is marked by storing a large positive value there
                //for now. If the intersection count is odd the point
                //is inside so a large negative value is stored
                if((intersectionCount % 2) == 0)
                {
                    xDistances.push_back(LARGE_PVE);
                }
                else
                {
                    xDistances.push_back(-LARGE_PVE);
                }
            }
            //Add the row of distance values that have been created
            //to the current 'z' row
            zDistances.push_back(xDistances);
        }
        //Finally add the container of z distance values to our class
        //member to create a 3x3 data structure containing the
        //distance values
        m_distancePoints.push_back(zDistances);
    }
}

void Mesh::calculateMAABB(float &xMin, float &xMax, float &yMin, float &yMax, float &zMin, float &zMax)
{
    //Iterate through every vertex that was loaded
    for(uint i = 0; i < m_verts.size(); ++i)
    {
        //Update the xMin and xMax values if need be
        if(m_verts[i].x() < xMin)
        {
            xMin = m_verts[i].x();
        }
        else if(m_verts[i].x() > xMax)
        {
            xMax = m_verts[i].x();
        }

        //Same with the yMin and yMax values
        if(m_verts[i].y() < yMin)
        {
            yMin = m_verts[i].y();
        }
        else if(m_verts[i].y() > yMax)
        {
            yMax = m_verts[i].y();
        }

        //And again with zMin and zMax
        if(m_verts[i].z() < zMin)
        {
            zMin = m_verts[i].z();
        }
        else if(m_verts[i].z() > zMax)
        {
            zMax = m_verts[i].z();
        }
    }

    //Add a margin of 3 * sphere radius to the bounding box
    float r3 = m_radius * 3;

    xMin -= r3;
    xMax += r3;

    yMin -= r3;
    yMax += r3;

    zMin -= r3;
    zMax +=r3;
}

void Mesh::createMAABB(QVector3D &xyz, QVector3D &Xyz, QVector3D &XyZ, QVector3D &xyZ, QVector3D &xYz, QVector3D &XYz, QVector3D &XYZ, QVector3D &xYZ)
{
    //The parameters are not very sensibly named but lowercase represents MINIMUM value and uppercase represent MAXIMUM

    //such that the bottom face is:
    //xyz--------------Xyz
    //  -                  -
    //  -                  -
    //  -                  -
    //xyZ--------------XyZ

    //and the top face is:
    //xYz--------------XYz
    //  -                  -
    //  -                  -
    //  -                  -
    //xYZ--------------XYZ

    float xMin, xMax;
    float yMin, yMax;
    float zMin, zMax;

    calculateMAABB(xMin, xMax, yMin, yMax, zMin, zMax);

    //The four vertices at the bottom of the box (clockwise)
    xyz = QVector3D(xMin, yMin, zMin);
    Xyz = QVector3D(xMax, yMin, zMin);
    XyZ = QVector3D(xMax, yMin, zMax);
    xyZ = QVector3D(xMin, yMin, zMax);

    //The four at the top (clockwise)
    xYz = QVector3D(xMin, yMax, zMin);
    XYz = QVector3D(xMax, yMax, zMin);
    XYZ = QVector3D(xMax, yMax, zMax);
    xYZ = QVector3D(xMin, yMax, zMax);
}

void Mesh::getPotentialTriangles(QVector2D point, std::vector<Intersections> &intersectionHolder)
{
    //Loop through the vertex indices (incrementing by 3
    //so actually looping through faces). Mesh was triangulated
    //on load so faces with 3 vertices can be assumed
    for(uint i = 0; i < m_meshIndex.size(); i+=3)
    {
        //Get the Y and Z coordinates of each face vertex. The testing ray is (1,0,0) so
        //we can ignore the X coordinate
        QVector2D A = QVector2D(m_verts[m_meshIndex[i]].y(), m_verts[m_meshIndex[i]].z());
        QVector2D B = QVector2D(m_verts[m_meshIndex[i + 1]].y(), m_verts[m_meshIndex[i + 1]].z());
        QVector2D C = QVector2D(m_verts[m_meshIndex[i + 1]].y(), m_verts[m_meshIndex[i + 2]].z());

        //Get the barycentric coordiantes of the point
        QVector3D bCentric = getBarycentricCoordinates(point, A, B, C);

        //Check if the bCentric coordinates indicate an intersection
        if(((0 <= bCentric.x()) && (bCentric.x() <= 1)) && ((0 <= bCentric.y()) && (bCentric.y() <= 1)) && ((0 <= bCentric.z()) && (bCentric.z() <= 1)))
        {
            //There is an intersection so create a new struct instance
            Intersections inter;

            //Store the bCentric data for later
            inter.baryCentric = bCentric;

            //Store the vertex indicies of the face intersected with
            inter.tri.A = m_verts[m_meshIndex[i]];
            inter.tri.A = m_verts[m_meshIndex[i + 1]];
            inter.tri.A = m_verts[m_meshIndex[i + 2]];

            //Finally check to see if the coordinates indicate:
            //      - Vertex intersection (2 coordinates = 0)
            //      - Edge intersection (1 coordinate = 0)
            //      - Face intersection (everything else)

            if(((bCentric.x() == 0) && (bCentric.y() == 0)) ||
                    ((bCentric.x() == 0) && (bCentric.z() == 0)) ||
                    ((bCentric.y() == 0) && (bCentric.z() == 0)))
            {
                //Set the intersection type accordingly
                inter.type = Vertex;
            }
            else if ((bCentric.x() == 0) ||
                     (bCentric.y() == 0) ||
                     (bCentric.z() == 0))
            {
                inter.type = Edge;
            }
            else
            {
                inter.type = Face;
            }

            //Finally add the intersection to the container
            //that was passed in
            intersectionHolder.push_back(inter);
        }
    }
}

QVector3D Mesh::getBarycentricCoordinates(QVector2D point, QVector2D A, QVector2D B, QVector2D C)
{
    QVector2D a(0,0);
    QVector2D b = QVector2D(B.x() - A.x(), B.y() - A.y());
    QVector2D c = QVector2D(C.x() - A.x(), C.y() - A.y());
    QVector2D p = QVector2D(point.x() - A.x(), point.y() - A.y());

    float d = b.x() * c.y() - c.y() * b.y();

    float u = (p.x() * (b.y() -c.y()) + p.y() * (c.x() - b.x()) + d) / d;
    float v = (p.x() * c.y() - p.y() * c.x()) / d;
    float w = (p.y() * b.x() - p.x() * b.y()) / d;

    return QVector3D(u, v, w);

}

QVector3D Mesh::calculateTriNorm(Triangle tri)
{
    //Calculate two edges of the triangle
    QVector3D e1 = tri.B - tri.A;
    QVector3D e2 = tri.C - tri.A;

    //Return the normalised cross product of these
    //edges (the triangle normal)
    return QVector3D::crossProduct(e1, e2).normalized();
}

void Mesh::packSpheres()
{
    //Set some variables for later use
    QVector3D middlePos;

    //Calculate the middle of the mesh
    for(uint i = 0; i < m_verts.size(); ++i)
    {
        middlePos += m_verts[i];
    }

    middlePos /= m_verts.size();

    //Create the start positions for the first 3 spheres
    QVector3D p0 = middlePos;
    QVector3D p1 = p0;
    QVector3D p2 = p0;
    p0[0] += m_radius;
    p1[0] -= m_radius;
    p2[2] += m_radius;

    p2[2] += m_radius * 0.75f;

    //Add these intial positions to the position container
    m_spherePositions.push_back(p0);
    m_spherePositions.push_back(p1);
    m_spherePositions.push_back(p2);
}
