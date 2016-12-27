#include "mesh.h"

#define LARGE_PVE 1000000000

void printVector(QVector3D p)
{
    qInfo()<<"Vec "<<p.x()<<' '<<p.y()<<' '<<p.z();
}

Mesh::Mesh(QVector4D colour)
{
    //Set the colour to the specified value
    m_colour = colour;

    m_radius = 0.25f;
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

void Mesh::preparePoints(QOpenGLShaderProgram &program)
{
    m_vaoP.create();
    m_vaoP.bind();

    m_vboP.create();
    m_vboP.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vboP.bind();

    m_vboP.allocate(&m_pointPositions[0],  (int)m_pointPositions.size() * sizeof(GLfloat) * 3);

    program.enableAttributeArray("vertexPos");
    program.setAttributeArray("vertexPos", GL_FLOAT, 0, 3);

    m_vboP.release();

    m_vaoP.release();
}

void Mesh::drawPoints()
{
    m_vaoP.bind();

    glDrawArrays(GL_POINTS, 0,  (int)m_pointPositions.size());

    m_vaoP.release();
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

void Mesh::calculateMAABB()
{
    float xMin = LARGE_PVE;
    float xMax = -LARGE_PVE;

    float yMin = LARGE_PVE;
    float yMax = -LARGE_PVE;

    float zMin = LARGE_PVE;
    float zMax = -LARGE_PVE;

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

    //Add a margin of 1 sphere radius to the bounding box

    xMin -=  m_radius *2;
    xMax += m_radius *2;

    yMin -= m_radius *2;
    yMax += m_radius *2;

    zMin -= m_radius *2;
    zMax += m_radius *2;

    m_meshAABB.xMin = xMin;
    m_meshAABB.xMax = xMax;
    m_meshAABB.yMin = yMin;
    m_meshAABB.yMax = yMax;
    m_meshAABB.zMin = zMin;
    m_meshAABB.zMax = zMax;
}

int Mesh::getIntersections(QVector3D p, QVector3D dir)
{
    //Initialise the intersection counter and a boolean for later
    int hitCount = 0;
    bool isOnFace = false;

    //Initialise two new vectors for containing values when an
    //intersection occurs on a vertex or edge
    std::vector<float> vertHits;
    std::vector<float> edgeHits;

    //Iterate through each face in the mesh
    for(uint i = 0; i < m_meshIndex.size(); i+=3)
    {
        //Get the vertex data from the face and add it to a triangle. The
        //mesh was triangulated upon load so faces with 3 vertices can be
        //assumed
        Triangle t;
        t.A = m_verts[m_meshIndex[i]];
        t.B = m_verts[m_meshIndex[i + 1]];
        t.C = m_verts[m_meshIndex[i + 2]];

        //Create a new ray instance with starting position at the point and
        //the passed in direction vector
        Ray test = {p, dir};

        //Calculate the vectors describing two of the face edges
        QVector3D e1 = t.B - t.A;
        QVector3D e2 = t.C - t.A;

        //By taking the cross product of these two edges a normal
        //for the face is obtained
        QVector3D N = QVector3D::crossProduct(e1, e2);

        //Take the cross between an edge and the ray direction
        QVector3D q = QVector3D::crossProduct(test.dir, e2);

        //By taking the dot product between this vector and the
        //other edge we can check to see if the face is actually
        //parallel to the ray.
        float a = QVector3D::dotProduct(q, e1);

        //Allow for a small error but if the absolute value of this
        //dot product is 0 we can ignore this face because it is
        //parallel
        if( (fabs(a) < 0.0001f))
        {
            continue;
        }

        //Next we do inside/outside tests with an edge
        QVector3D s = p - t.A;
        QVector3D r = QVector3D::crossProduct(s, e1);

        //Calculate the area of the triangle formed by the
        //intersection point and two verts relative to the
        //entire face
        float u = QVector3D::dotProduct(s, q) / a;

        //If the area is less than 0 or greater than 1 the
        //intersection point is not inside the current face
        if( u < 0.0f || u > 1.0f)
        {
            continue;
        }

        //Do the same for a new edge
        float v = QVector3D::dotProduct(r, test.dir) / a;

        //Do the same inside/outside test
        if( v < 0.0f || u + v > 1.0f)
        {
            continue;
        }

        //Calculate the t parameter of the intersection point
        //equation:
        //      Intersection = StartPoint + t * RayDirection
        float tParam = QVector3D::dotProduct(e2, r) / a;

        //If the parameter is 0 this means the ray actually
        //starts on the current face, mark this and break
        //from the for loop
        if(tParam == 0.0f)
        {
            isOnFace = true;
            break;
        }

        //Check that the t parameter is greater than 0, if it
        //isn't the face can be ignored because the intersection
        //point is behind the ray origin
        if((tParam > 0.0f))
        {
            //Calculate the final barycentric coordinate
            float w = 1.0f - u - v;

            //If either of the two coordinates are 0 this
            //means the intersection is with a vertex.
            //By checking the dot product of each hit
            //triangle's normal with the ray direction we
            //can determine if the ray is leaving or entering
            //the mesh.
            if(((u == 0) && (v == 0)) ||
                    ((u == 0) && (w == 0)) ||
                    ((v == 0) && (w == 0)))
            {
                //As the current direction is (1,0,0)
                //the x component of the normal is the only
                //value that matters
                vertHits.push_back(N.x());
                continue;
            }
            //At this point we know that 2 of the coordinates
            //do not equal 0, so check if 1 of them does
            //because this signifies an edge hit
            else if((u == 0) || (v == 0) || (w == 0))
            {
                edgeHits.push_back(N.x());
                continue;
            }

            //If all the tests have passed the intersection is
            //on a face and the hit counter can be incremented
            hitCount++;
        }
    }

    //Set two variables, one for signifying whether the hit
    //is valid and one for storing the sign of the data
    bool isHit = true;
    float sign = 0;

    //Check if there were any vertex hits
    if(vertHits.size() > 0)
    {
        //Check the sign of the normal of the first hit
        //triangle
        vertHits[0] < 0.0f ? sign = -1 : sign = 1;

        //Iterate through the data, if any of the signs
        //are different then the hit can be ignored
        for(uint i = 0; i < vertHits.size(); ++i)
        {
            if((vertHits[i] < 0 && sign == 1) ||
                    (vertHits[i] > 0 && sign == -1))
            {
                isHit = false;
                break;
            }
        }

        //Check for a valid hit and increment the hit counter
        if(isHit)
        {
            hitCount++;
        }
    }

    //Reset the variables for the edge checks
    isHit = true;
    sign = 0;

    //Do the same process for any possible edge
    //intersections
    if(edgeHits.size() > 0)
    {
        edgeHits[0] < 0.0f ? sign = -1 : sign = 1;

        for(uint i = 0; i < edgeHits.size(); ++i)
        {
            if((edgeHits[i] < 0 && sign == 1) ||
                    (edgeHits[i] > 0 && sign == -1))
            {
                isHit = false;
                break;
            }
        }

        if(isHit)
        {
            hitCount++;
        }
    }

    //Finally see if there were any intersections
    //on a face. If there were and the hit count is
    //currently even (signifying outside the mesh)
    //then make sure the hit counter is odd
    if(isOnFace && ((hitCount % 2) == 0))
    {
        hitCount++;
    }

    //Finally return the number of intersections
    return hitCount;
}

bool Mesh::triangleEquals(Triangle a, Triangle b)
{


    if(((a.A.x() == b.A.x() && a.A.y() == b.A.y() && a.A.z() == b.A.z()) ) &&
        ((a.B.x() == b.B.x() && a.B.y() == b.B.y() && a.B.z() == b.B.z()) ) &&
        ((a.C.x() == b.C.x() && a.C.y() == b.C.y() && a.C.z() == b.C.z()) ))
    {
        return true;
    }

    return false;
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

//-------------------------SPHERE PACKING STUFF ----------------------------------//

void Mesh::generateDistanceField()
{
    //Intialise the bounding box minimum and maximum values
    //to large numbers to ensure no errors when calculating the
    //Margined Axis Aligned Bouding Box (MAABB)

    //Calculate the MAABB
    calculateMAABB();

    m_boxResolution = 1.0f;

    float deltaPercent = (m_boxResolution/(m_meshAABB.yMax - m_meshAABB.yMin)) * 100;
    float totalPercent = 0.0f;

    Triangle defaultTri;

    qInfo() << "Creating distance point grid\n";

    //Iterate through every unit point in the MAABB, essentially creating a 3D grid
    for(float y = m_meshAABB.yMin; y <= m_meshAABB.yMax; y+=m_boxResolution)
    {
        std::vector< std::vector< int> > zDistances;
        std::vector< std::vector<Triangle> > zTriangles;

        for(float z = m_meshAABB.zMin; z <= m_meshAABB.zMax; z+=m_boxResolution)
        {
            std::vector<int> xDistances;
            std::vector<Triangle> xTriangles;
            for(float x = m_meshAABB.xMin; x <= m_meshAABB.xMax; x+=m_boxResolution)
            {
                //Create a new point and a container to store intersections in

                //Initialise the intersection count for this point to 0
                int intersectionCount = 0;

                //Get the number of intersections between a ray starting at the current
                //point with a direction vector of (1,0,0) and the mesh
                intersectionCount = getIntersections(QVector3D(x,y,z), QVector3D(1,0,0));

                //Do a simple test to see if the number of intersections is odd or not,
                //if it is the point is inside the mesh. This is marked with a large negative
                //number, points outside are marked with a large positive number
                if(((intersectionCount % 2) == 0))
                {
                    xDistances.push_back(LARGE_PVE);
                }
                else
                {
//                    m_pointPositions.push_back(QVector3D(x, y, z));
                    xDistances.push_back(-LARGE_PVE);
                }


                xTriangles.push_back(defaultTri);

            }
            //Add the row of distance values that have been created
            //to the current 'z' row
            zDistances.push_back(xDistances);
            zTriangles.push_back(xTriangles);
        }
        //Finally add the container of z distance values to our class
        //member to create a 3x3 data structure containing the
        //distance values
        m_distancePoints.push_back(zDistances);
        m_distanceTriangles.push_back(zTriangles);

        totalPercent += deltaPercent;
        qInfo() << "Distance points are "<<(int)totalPercent<<"% done\n";
    }

    qInfo() << "Initial distance point grid creation completed\n";

    for(uint i = 0; i < m_meshIndex.size(); i += 3)
    {
        //Create the shells for the distance field. This is done
        //by taking each triangle face and extruding it into a prism
        //using the normal of the triangle face.
        Triangle t;
        t.A = m_verts[m_meshIndex[i]];
        t.B = m_verts[m_meshIndex[i + 1]];
        t.C = m_verts[m_meshIndex[i + 2]];

        //The constructor generates the prism and a bounding box
        //for later
        Prism p(t);

        //Add this new prism to the shell container
        m_shell.push_back(p);
    }

    qInfo() << "Shell size: "<<m_shell.size()<<'\n';

    qInfo()<<"Distance field creation is complete\n";

    int ySize =  (int)m_distancePoints.size();

    float  y = m_meshAABB.yMin;

    for(int yI = 0; yI < ySize; yI++)
    {
        int zSize =  (int)m_distancePoints[yI].size();

        float z = m_meshAABB.zMin;

        for(int zI = 0; zI < zSize; zI++)
        {
            int xSize =  (int)m_distancePoints[yI][zI].size();

            float x = m_meshAABB.xMin;

            for(int xI = 0; xI < xSize; xI++)
            {
                //This time we also iterate through the mesh shell
                for(uint i = 0; i < m_shell.size(); ++i)
                {
                    //Store the current prism locally
                    Prism prism = m_shell[i];

                    //And also get the current grid point
                    QVector3D curPoint(x, y, z);

                    //Check if the grid point is within the vaccinity
                    //of the prism's bounding box
                    if(prism.bBoxContains(curPoint))
                    {

                        //Calcualte the closest point on the triangle face
                        //the prism was formed from to the grid point
                        QVector3D closestPoint = prism.checkWhere(curPoint);

                        //Then get the distance to that point
                        float distance = (closestPoint - curPoint).length();

                        //Check what the current value for 'closest distance'
                        //is
                        if(fabs(m_distancePoints[yI][zI][xI]) > fabs(distance))
                        {
                            //The distance is a closer one so we need to get the sign
                            //of the distance (simple dot product)
                            int sign = 0;
                            QVector3D::dotProduct(closestPoint, prism.getNormal()) < 0 ? sign = -1 : sign = 1;

                            //The distance is converted to a signed value
                            distance *= sign;

                            //A check is done to see if the currently stored triangle for this point
                            //is the default triangle inserted in the points generation stage
                            if(triangleEquals(m_distanceTriangles[yI][zI][xI], defaultTri) == false)
                            {
                                //The centers of the currently marked triangle and the potential new
                                //one are calculated
                                QVector3D aCenter = (m_distanceTriangles[yI][zI][xI].A +
                                                                m_distanceTriangles[yI][zI][xI].B +
                                                                m_distanceTriangles[yI][zI][xI].C) / 3.0f;

                                QVector3D bCenter = (prism.getTriangle().A +
                                                                prism.getTriangle().B +
                                                                prism.getTriangle().C) / 3.0f;

                                //The vector between the two centers is next calculated
                                QVector3D connectVector = bCenter - aCenter;

                                //By checking the dot of this connection vector with one of the triangle
                                //normals the check for concave/convex faces is made
                                float connectType = QVector3D::dotProduct(calculateTriNorm(m_distanceTriangles[yI][zI][xI]), connectVector);

                                if(connectType >= 0.0f)
                                {
                                    //Two faces are concave, make sure the signs are different
                                    //distance change is ONLY from negative to positive
                                    if((distance > 0.0f) && (m_distancePoints[yI][zI][xI] <= 0.0f))
                                    {
                                        //Set the new distance
                                        m_distancePoints[yI][zI][xI] = distance;

                                        //The triangle that forms the closest distance is also marked
                                        m_distanceTriangles[yI][zI][xI] = prism.getTriangle();
                                    }
                                    else if((distance <= 0.0f && m_distancePoints[yI][zI][xI] <= 0.0f) ||
                                             (distance > 0.0f && m_distancePoints[yI][zI][xI] > 0.0f))
                                    {
                                        //The distances have the same sign, set the new distance
                                        m_distancePoints[yI][zI][xI] = distance;

                                        //And mark the triangle
                                        m_distanceTriangles[yI][zI][xI] = prism.getTriangle();
                                    }
                                }

                                if(connectType < 0.0f)
                                {
                                    //Two faces are convex. Ensure if the distances have different signs
                                    //change can only be made going from positive to negative
                                    if((distance <= 0.0f) && (m_distancePoints[yI][zI][xI] > 0.0f))
                                    {
                                        //Same storing process
                                        m_distancePoints[yI][zI][xI] = distance;
                                        m_distanceTriangles[yI][zI][xI] = prism.getTriangle();
                                    }
                                    else if((distance <= 0.0f && m_distancePoints[yI][zI][xI] <= 0.0f) ||
                                             (distance > 0.0f && m_distancePoints[yI][zI][xI] > 0.0f))
                                    {
                                        //Check again for distance with the same sign
                                        m_distancePoints[yI][zI][xI] = distance;
                                        m_distanceTriangles[yI][zI][xI] = prism.getTriangle();
                                    }
                                }

                            }
                            else
                            {
                                m_distancePoints[yI][zI][xI] = distance;

                                //The triangle that forms the closest distance is also marked
                                m_distanceTriangles[yI][zI][xI] = prism.getTriangle();
                            }
                        }
                    }
                }

                x += m_boxResolution;

            }
            z += m_boxResolution;

        }
        y += m_boxResolution;
    }

    y = m_meshAABB.yMin;

    for(int yI = 0; yI < ySize; yI++)
    {
        int zSize =  (int)m_distancePoints[yI].size();

        float z = m_meshAABB.zMin;

        for(int zI = 0; zI < zSize; zI++)
        {
            int xSize =  (int)m_distancePoints[yI][zI].size();

            float x = m_meshAABB.xMin;

            for(int xI = 0; xI < xSize; xI++)
            {
                if(m_distancePoints[yI][zI][xI] != -LARGE_PVE && m_distancePoints[yI][zI][xI] != LARGE_PVE)
                {
                    m_pointPositions.push_back(QVector3D(x, y, z));
                }

                x += m_boxResolution;
            }
            z += m_boxResolution;

        }
        y += m_boxResolution;
    }

    qInfo()<<"Shell creation finished";

}

void Mesh::packSpheres()
{
    //Set some variables for later use
    QVector3D middlePos;

    //Calculate the middle of the mesh
    for(uint i = 0; i < m_verts.size(); ++i)
    {
        middlePos += m_verts[i];
        middlePos.setY(middlePos.y() + 5);
    }

    middlePos /= m_verts.size();
    qInfo()<<"Mesh COM: "<<middlePos.x()<<' '<<middlePos.y()<<' '<<middlePos.z()<<'\n';


    //Create the start positions for the first 3 spheres
    QVector3D p0 = middlePos;
    QVector3D p1 = p0;
    QVector3D p2 = p0;
    p0[0] += m_radius;
    p1[0] -= m_radius;
    p2[1] += m_radius;

    p2[2] += m_radius * 0.75f;

    //Add these intial positions to the position container
    m_spherePositions.push_back(p0);
    m_spherePositions.push_back(p1);
    m_spherePositions.push_back(p2);

    std::vector<QVector3D> frontQueue;

    frontQueue.push_back(p1);
    frontQueue.push_back(p0);
    frontQueue.push_back(p2);

    int count = 0;

    int sphereIndex = 0;

    while(frontQueue.size() != 0)
    {
        if(frontQueue.size() >= 100)
        {
            qWarning()<<"Recursion guard reached\n";
            break;
        }

        QVector3D currentSphere = frontQueue[0];
        BBox neighbourhood = makeNeighbourhood(currentSphere);

        std::vector<QVector3D> neighbors;

        for(uint i = 0; i < frontQueue.size(); ++i)
        {
            if(i == 0)
            {
                continue;
            }
            else
            {
                if(bBoxContains(neighbourhood, currentSphere))
                {
                    neighbors.push_back(frontQueue[i]);
                }
            }
        }

        std::vector<QVector3D> candidatePoints;

        for(uint i = 0; i < neighbors.size(); ++i)
        {
            for(uint j = 0; j < neighbors.size(); ++j)
            {
                if(i == j)
                {
                    continue;
                }

                QVector3D validPoint1;
                QVector3D validPoint2;
                HaloIntersections pointType;

                haloIntersection(currentSphere, neighbors[i], neighbors[j],
                                        validPoint1, validPoint2,
                                        pointType);

                if(pointType != NoHit)
                {
                    candidatePoints.push_back(validPoint1);

                    if(pointType == TwoHits)
                    {
                        candidatePoints.push_back(validPoint2);
                    }
                }
            }
        }



        validatePoints(candidatePoints);

        if(candidatePoints.size() > 0)
        {
            int bestPoint = getBestPoint(currentSphere, candidatePoints);

            m_spherePositions.push_back(candidatePoints[bestPoint]);
            frontQueue.push_back( candidatePoints[bestPoint]);
        }

        if(candidatePoints.size() < 2)
        {
            frontQueue.erase(frontQueue.begin());

            sphereIndex++;
        }

        count++;
    }

    for(uint i =0; i < m_spherePositions.size(); ++i)
    {
        m_spherePositions[i] += QVector3D(0,10,0);
    }

    qInfo()<<"Finished packing spheres";
}

BBox Mesh::makeNeighbourhood(QVector3D p)
{
    BBox nBBox;

    float margin = 5 * m_radius;
    //THIS MIGHT NEED CHANGING?? PAGE 4
    nBBox.xMin = p.x() - margin;
    nBBox.xMax = p.x() + margin;

    nBBox.yMin = p.y() - margin;
    nBBox.yMax = p.y() + margin;

    nBBox.zMin = p.z() - margin;
    nBBox.zMax = p.z() + margin;

    return nBBox;
}

bool Mesh::bBoxContains(BBox box, QVector3D point)
{
    if((point.x() >= box.xMin && point.x() <= box.xMax) &&
        (point.y() >= box.yMin && point.y() <= box.yMax) &&
        (point.z() >= box.zMin && point.z() <= box.zMax))
    {
        return true;
    }

    return false;
}

void Mesh::haloIntersection(QVector3D a, QVector3D b, QVector3D c,
                                        QVector3D &hit1, QVector3D &hit2,
                                        HaloIntersections &intersectionType)
{
    float dist = (a - b).length();

    float haloRadius = m_radius + m_radius;

    if((dist + haloRadius) < haloRadius)
    {
        intersectionType = NoHit;
        return;
    }

    float alpha = 0.5;//(dist * dist);

    QVector3D circleCenter = a + alpha*(b - a);

    float circleRadius = sqrt((haloRadius * haloRadius) - ((alpha * dist) * (alpha * dist)));

    QVector3D circleN = (a - b).normalized();

    float smallestComponent = std::min({ circleN.x(), circleN.y(), circleN.z() });

    QVector3D cross;
    QVector3D u;
    QVector3D v;

    if(circleN.x() == smallestComponent)
    {
        cross = QVector3D::crossProduct(QVector3D(1.0f, 0.0f, 0.0f), circleN);
    }
    else if(circleN.y() == smallestComponent)
    {
        cross = QVector3D::crossProduct(QVector3D(0.0f, 1.0f, 0.0f), circleN);
    }
    else if(circleN.z() == smallestComponent)
    {
        cross = QVector3D::crossProduct(QVector3D(0.0f, 0.0f, 1.0f), circleN);
    }
    else
    {
        qWarning() <<"Smallest component wasnt in the intersection normal\n";
        exit(1);
    }

    u = cross.normalized();

    v = QVector3D::crossProduct(u, circleN);
    v.normalize();

    float dotDist = QVector3D::dotProduct(circleN, circleCenter - c);

    if(fabs(dotDist) > haloRadius)
    {
        intersectionType = NoHit;
        return;
    }

    QVector3D circleCenter2 = c + (dotDist * circleN);

    if(dotDist == haloRadius)
    {
        intersectionType = OneHit;
        return;
    }

    intersectionType = TwoHits;

    QVector3D circleN2 = QVector3D::crossProduct(circleCenter2 - circleCenter, circleN).normalized();

    hit1 = circleCenter - (circleN2 * circleRadius);
    hit2 = circleCenter + (circleN2 * circleRadius);


//    float separationSQRD = (a - b).lengthSquared();

//    float alpha = (1.0f - (m_radius * m_radius) - (m_radius * m_radius) / separationSQRD) / 2.0f;

//    QVector3D intCenter = ((1 - alpha) * a) + (alpha * b);

//    float intRadius = sqrt((separationSQRD - (m_radius * m_radius)));

//    QVector3D intNormal = (a - b).normalized();

//    float smallestComponent = std::min({ intNormal.x(), intNormal.y(), intNormal.z() });

//    QVector3D cross;
//    QVector3D u;
//    QVector3D v;

//    if(intNormal.x() == smallestComponent)
//    {
//        cross = QVector3D::crossProduct(intNormal, QVector3D(1.0f, 0.0f, 0.0f));
//    }
//    else if(intNormal.y() == smallestComponent)
//    {
//        cross = QVector3D::crossProduct(intNormal, QVector3D(0.0f, 1.0f, 0.0f));
//    }
//    else if(intNormal.z() == smallestComponent)
//    {
//        cross = QVector3D::crossProduct(intNormal, QVector3D(0.0f, 0.0f, 1.0f));
//    }
//    else
//    {
//        qWarning() <<"Smallest component wasnt in the intersection normal\n";
//        exit(1);
//    }

//    u = cross.normalized();

//    v = QVector3D::crossProduct(u, intNormal);
//    v.normalize();

//    float distance = QVector3D::dotProduct(c - intCenter, intNormal);

//    if(fabs(distance) > m_radius)
//    {
//        intersectionType = NoHit;
//        return;
//    }

//    QVector3D int2Center = c - (distance * intNormal);
//    float int2Radius = sqrt((m_radius * m_radius) - (distance * distance));

//    float finalCheck = m_radius + m_radius - (int2Center - intCenter).length();

//    if(finalCheck < 0.0f)
//    {
//        intersectionType = NoHit;
//        return;
//    }

//    if(finalCheck == 0.0f)
//    {
//        intersectionType = OneHit;
//        hit1 = intCenter + ((int2Center - intCenter).normalized() * m_radius);
//        return;
//    }

//    alpha = (1.0f - (((int2Radius * int2Radius) - (intRadius * intRadius)) / ((int2Center - intCenter).length()))) / 2.0f;

//    QVector3D int3Center = ((1 - alpha) * intCenter) + (alpha * int2Center);
//    QVector3D circleSeparation = (int3Center - intCenter);

//    float circleHeight = sqrt((circleSeparation.length() * circleSeparation.length()) - (m_radius * m_radius));
//    QVector3D finalCross = QVector3D::crossProduct(intNormal, circleSeparation);

//    intersectionType = TwoHits;

//    hit1 = int3Center + (circleHeight * (finalCross / finalCross.length()));
//    hit2 = int3Center - (circleHeight * (finalCross / finalCross.length()));

}

void Mesh::validatePoints(std::vector<QVector3D> &points)
{
    uint numPoints =  (uint)points.size();
    std::vector<QVector3D> acceptedPoints;

    for(uint i = 0; i < numPoints; ++i)
    {
        float distance = interpolateTrilinear(points[i]);

        if(distance <= m_radius)
        {
            acceptedPoints.push_back(points[i]);
        }
    }

    points = acceptedPoints;

    std::vector<QVector3D> aP2;

    std::vector<int> pointsToRemove;

    numPoints =  (uint)points.size();

    for(uint i = 0; i <m_spherePositions.size(); ++i)
    {
        for(uint j = 0; j <  numPoints; ++j)
        {
            if(std::find(pointsToRemove.begin(), pointsToRemove.end(), j) == pointsToRemove.end())
            {
                float dist = (m_spherePositions[i] - points[j]).length();

                if(dist <= m_radius * 1.75f)
                {
                    pointsToRemove.push_back(j);
                }
            }
        }
    }

    for(uint i = 0; i < points.size(); ++i)
    {
        if(std::find(pointsToRemove.begin(), pointsToRemove.end(), i) == pointsToRemove.end())
        {
            aP2.push_back(points[i]);
        }
    }

    points = aP2;

}

float Mesh::interpolateLinear(float x, float x1, float x2, float c00, float c01)
{
    return ( ((x2 - x) / (x2 - x1)) * c00 ) + ( ((x - x1) / (x2 - x1)) * c01 );
}

float Mesh::interpolateTrilinear(QVector3D p)
{
    int yIndex = 0;

    int zIndex = 0;

    int xIndex = 0;

    for(float y = m_meshAABB.yMin; y < m_meshAABB.yMax; y += m_boxResolution)
    {
        //Reset the indices used for accessing the grid point container
        zIndex = 0;

        for(float z = m_meshAABB.zMin; z < m_meshAABB.zMax; z += m_boxResolution)
        {
            xIndex = 0;

            for(float x = m_meshAABB.xMin; x < m_meshAABB.xMax; x += m_boxResolution)
            {
                if((p.x() >= x && p.x() <= (x + m_boxResolution)) &&
                    (p.y() >= y && p.y() <= (y + m_boxResolution)) &&
                    (p.z() >= z && p.z() <= (z + m_boxResolution)) )
                {
                    float x00 = interpolateLinear(p.x(), x, (x + m_boxResolution),
                                                            m_distancePoints[yIndex][zIndex][xIndex],
                                                            m_distancePoints[yIndex][zIndex][xIndex + 1]);

                    float x10 = interpolateLinear(p.x(), x, (x + m_boxResolution),
                                                            m_distancePoints[yIndex + 1][zIndex][xIndex],
                                                            m_distancePoints[yIndex + 1][zIndex][xIndex + 1]);

                    float x01 = interpolateLinear(p.x(), x, (x + m_boxResolution),
                                                            m_distancePoints[yIndex][zIndex + 1][xIndex],
                                                            m_distancePoints[yIndex][zIndex + 1][xIndex + 1]);

                    float x11 = interpolateLinear(p.x(), x, (x + m_boxResolution),
                                                            m_distancePoints[yIndex + 1][zIndex + 1][xIndex],
                                                            m_distancePoints[yIndex + 1][zIndex + 1][xIndex + 1]);

                    float y0 = interpolateLinear(p.y(), y, (y + m_boxResolution),
                                                             x00,
                                                             x01);

                    float y1 = interpolateLinear(p.y(), y, (y + m_boxResolution),
                                                             x10,
                                                             x11);

                    float value = interpolateLinear(p.z(), z, (z + m_boxResolution),
                                                                y0,
                                                                y1);



                    return value;
                }

                //Increment the indices used for accesing the grid points
                xIndex++;
            }
            zIndex++;
        }
        yIndex++;
    }

//    qWarning()<<"Point not inside Mesh AABB, triliear interpolation didn't happen\n"
//                    "Point: "<<p.x()<<", "<<p.y()<<", "<<p.z()<<'\n';

    return 100000000.0f;

}

int Mesh::getBestPoint(QVector3D currentSphere, std::vector<QVector3D> points)
{
    float distance = LARGE_PVE;
    int index = 0;

    for(uint i = 0; i < points.size(); ++i)
    {
        float tmpDistance = (points[i] - currentSphere).length();

        if(tmpDistance < distance)
        {
            distance = tmpDistance;
            index = i;
        }
    }

    return index;
}

void Mesh::getCloseSpheres(uint sphereIndex, std::vector<QVector3D> &positions, std::vector< std::pair<uint, uint> > &pairs)
{
    QVector3D curSphere = m_spherePositions[sphereIndex];

    float margin = 10 * m_radius;

    for(uint i = 0; i < m_spherePositions.size(); ++i)
    {
        //Ignore the sphere we're testing
        if(i == sphereIndex)
        {
            continue;
        }

        //Get a sphere from the list of positions
        QVector3D testSphere = m_spherePositions[i];

        //Check whether the x component is within range
        float xMin = curSphere.x() - margin;
        float xMax = curSphere.x() + margin;

        //If it isn't we can short circuit and skip the rest
        if(testSphere.x() < xMin || testSphere.x() > xMax)
        {
            continue;
        }

        //Do the same for the y component
        float yMin = curSphere.y() - margin;
        float yMax = curSphere.y() + margin;

        if(testSphere.y() < yMin || testSphere.y() > yMax)
        {
            continue;
        }

        //And finally test the z component
        float zMin = curSphere.z() - margin;
        float zMax = curSphere.z() + margin;

        if(testSphere.z() < zMin || testSphere.z() > zMax)
        {
            continue;
        }

        //The point is within our bounding box, get
        //the actual distance separating the current
        //sphere and this sphere
        float distance = (testSphere - curSphere).length();

        //Check if the distance is acceptable
        if(distance <= margin)
        {
            auto pairPosition = std::find_if(pairs.begin(), pairs.end(), FindPair(sphereIndex, i));

            //This indicates the pair was not found
            if(pairPosition == pairs.end())
            {
                //If it is then add it to the list of accepted positions
                positions.push_back(testSphere);
                pairs.push_back(std::make_pair(sphereIndex, i));
            }
            else
            {
//                qInfo()<<"Pair: "<<sphereIndex<<", "<<i<<" was already found";
            }
        }
    }

}



