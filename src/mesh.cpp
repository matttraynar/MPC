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

void Mesh::preparePoints(QOpenGLShaderProgram &program)
{
    m_vaoP.create();
    m_vaoP.bind();

    m_vboP.create();
    m_vboP.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vboP.bind();

    m_vboP.allocate(&m_pointPositions[0], m_pointPositions.size() * sizeof(GLfloat) * 3);

    program.enableAttributeArray("vertexPos");
    program.setAttributeArray("vertexPos", GL_FLOAT, 0, 3);

    m_vboP.release();

    m_vaoP.release();
}

void Mesh::drawPoints()
{
    m_vaoP.bind();

    glDrawArrays(GL_POINTS, 0, m_pointPositions.size());

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

    std::cout<<"xMin: "<<xMin<<" xMax: "<<xMax<<'\n';
    std::cout<<"yMin: "<<yMin<<" yMax: "<<yMax<<'\n';
    std::cout<<"zMin: "<<zMin<<" zMax: "<<zMax<<'\n';

    //Iterate through every unit point in the MAABB, essentially creating a 3D grid
    for(float y = (float)yMin; y <= (float)yMax; y+=.5f)
    {
        std::vector< std::vector< int> > zDistances;

        for(float z = (float)zMin; z <= (float)zMax; z+=.5f)
        {
            std::vector<int> xDistances;

            for(float x = (float)xMin; x <= (float)xMax; x+=.5f)
            {
                //Create a new point and a container to store intersections in

                //Initialise the intersection count for this point to 0
                int intersectionCount = 0;

                //Look for triangles which have a 2D intersection (so might have a 3D one)
                //getPotentialTriangles(QVector3D(x, y, z), pointHits);
                intersectionCount = getIntersections(QVector3D(x,y,z), QVector3D(1,0,0));

                //After all that we can check the intersection count.
                //An even number means the point is outside the mesh,
                //this is marked by storing a large positive value there
                //for now. If the intersection count is odd the point
                //is inside so a large negative value is stored

                if(((intersectionCount % 2) == 0) || (intersectionCount == 0))
                {
                    xDistances.push_back(LARGE_PVE);
                }
                else
                {
                    m_pointPositions.push_back(QVector3D(x, y, z));
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

    //Add a margin of 1 sphere radius to the bounding box

    xMin -=  m_radius;
    xMax += m_radius;

    yMin -= m_radius;
    yMax += m_radius;

    zMin -= m_radius;
    zMax += m_radius;
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

int Mesh::getIntersections(QVector3D p, QVector3D dir)
{
    int hitCount = 0;
    bool isOnFace = false;

    std::vector<float> vertHits;
    std::vector<float> edgeHits;

    for(uint i = 0; i < m_meshIndex.size(); i+=3)
    {
        Triangle t = {m_verts[m_meshIndex[i]], m_verts[m_meshIndex[i + 1]], m_verts[m_meshIndex[i + 2]]};

        Ray test = {p, dir};
        QVector3D end = p + (test.dir * 50);

        QVector3D e1 = t.B - t.A;
        QVector3D e2 = t.C - t.A;

        QVector3D N = QVector3D::crossProduct(e1, e2);

        QVector3D q = QVector3D::crossProduct(test.dir, e2);
        float a = QVector3D::dotProduct(q, e1);

        if( (fabs(a) < 0.0001f))
        {
            continue;
        }

        QVector3D s = p - t.A;
        QVector3D r = QVector3D::crossProduct(s, e1);

        float u = QVector3D::dotProduct(s, q) / a;

        if( u < 0.0f || u > 1.0f)
        {
            continue;
        }

        float v = QVector3D::dotProduct(r, test.dir) / a;

        if( v < 0.0f || u + v > 1.0f)
        {
            continue;
        }

        float tParam = QVector3D::dotProduct(e2, r) / a;

        if(tParam == 0.0f)
        {
            isOnFace = true;
            break;
        }

        if((tParam > 0.0f))
        {
            float w = 1.0f - u - v;

            if(((u == 0) && (v == 0)) ||
                    ((u == 0) && (w == 0)) ||
                    ((v == 0) && (w == 0)))
            {
                vertHits.push_back(N.x());
                continue;
            }
            else if((u == 0) || (v == 0) || (w == 0))
            {
                edgeHits.push_back(N.x());
                continue;
            }

            hitCount++;
        }
    }

    bool isHit = true;
    float sign = 0;

    if(vertHits.size() > 0)
    {
        vertHits[0] < 0.0f ? sign = -1 : sign = 1;

        for(uint i = 0; i < vertHits.size(); ++i)
        {
            if((vertHits[i] < 0 && sign == 1) ||
                    (vertHits[i] > 0 && sign == -1))
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

    isHit = true;
    sign = 0;

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

    if(isOnFace && ((hitCount % 2) == 0))
    {
        hitCount++;
    }

    return hitCount;
}

void Mesh::getPotentialTriangles(QVector3D point, std::vector<Intersections> &intersectionHolder)
{
    //Loop through the vertex indices (incrementing by 3
    //so actually looping through faces). Mesh was triangulated
    //on load so faces with 3 vertices can be assumed
    for(uint i = 0; i < m_meshIndex.size(); i+=3)
    {
        QVector3D A = m_verts[m_meshIndex[i]];
        QVector3D B = m_verts[m_meshIndex[i + 1]];
        QVector3D C = m_verts[m_meshIndex[i + 2]];

        Triangle T = {A, B, C};

//        A.setX(point.x());
//        B.setX(point.x());
//        C.setX(point.x());

        if(pointInTriBBox(point, T))
        {

            //Get the barycentric coordiantes of the point
            QVector3D bCentric = getBarycentricCoordinates(point, A, B, C);

            //Check if the bCentric coordinates indicate an intersection
//            if(  ((0 <= bCentric.x()) && (bCentric.x() <= 1)) &&
//                 ((0 <= bCentric.y()) && (bCentric.y() <= 1)) &&
//                 ((0 <= bCentric.z()) && (bCentric.z() <= 1))    )
//            {
                //There is an intersection so create a new struct instance
                Intersections inter;

                //Store the bCentric data for later
                inter.baryCentric = bCentric;

                //Store the vertex indicies of the face intersected with
                inter.tri = T;
//                inter.tri.A = m_verts[m_meshIndex[i]];
//                inter.tri.B = m_verts[m_meshIndex[i + 1]];
//                inter.tri.C = m_verts[m_meshIndex[i + 2]];

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
                inter.pos = (inter.tri.A * bCentric.x()) + (inter.tri.B * bCentric.y()) * (inter.tri.C * bCentric.z());

                intersectionHolder.push_back(inter);
//            }
        }
        else
        {
            continue;
        }
    }
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
        if(  ((0 <= bCentric.x()) && (bCentric.x() <= 1)) &&
             ((0 <= bCentric.y()) && (bCentric.y() <= 1)) &&
             ((0 <= bCentric.z()) && (bCentric.z() <= 1))    )
        {
            //There is an intersection so create a new struct instance
            Intersections inter;

            //Store the bCentric data for later
            inter.baryCentric = bCentric;

            //Store the vertex indicies of the face intersected with
            inter.tri.A = m_verts[m_meshIndex[i]];
            inter.tri.B = m_verts[m_meshIndex[i + 1]];
            inter.tri.C = m_verts[m_meshIndex[i + 2]];

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

            inter.pos = (inter.tri.A * bCentric.x()) + (inter.tri.B * bCentric.y()) * (inter.tri.C * bCentric.z());
            intersectionHolder.push_back(inter);
        }
    }
}

float Mesh::getXFromVec(QVector3D a, QVector3D b)
{
    return ((a + b)/2.0f).x();
}

bool Mesh::pointInTriBBox(QVector3D p, Triangle t)
{
    float xMin = LARGE_PVE;
    float xMax = -LARGE_PVE;

    float yMin = LARGE_PVE;
    float yMax = -LARGE_PVE;

    float zMin = LARGE_PVE;
    float zMax = -LARGE_PVE;

    std::vector<QVector3D> triangle = {t.A, t.B, t.C};

    for(uint i = 0; i < 3; ++i)
    {
        if(triangle[i].x() < xMin)
        {
            xMin = triangle[i].x();
        }

        if(triangle[i].x() > xMax)
        {
            xMax = triangle[i].x();
        }

        if(triangle[i].y() < yMin)
        {
            yMin = triangle[i].y();
        }

        if(triangle[i].y() > yMax)
        {
            yMax = triangle[i].y();
        }

        if(triangle[i].z() < zMin)
        {
            zMin = triangle[i].z();
        }

        if(triangle[i].z() > zMax)
        {
            zMax = triangle[i].z();
        }
    }

    xMin -= 0.1f;
    xMax += 0.1f;

    yMin -=0.1f;
    yMax += 0.1f;

    zMin -= 0.1f;
    zMax += 0.1f;

    if(p.x() < xMin || p.x() > xMax ||
        p.y() < yMin || p.y() > yMax ||
        p.z() < zMin || p.z() > zMax)
    {
        return false;
    }

    return true;
}

QVector3D Mesh::getBarycentricCoordinates(QVector3D point, QVector3D A, QVector3D B, QVector3D C)
{
    QVector3D noHit(LARGE_PVE, LARGE_PVE, LARGE_PVE);

    QVector3D v0 = C - A;
    QVector3D v1 = B - A;
    QVector3D v2 = point - A;

    float dot00 = QVector3D::dotProduct(v0, v0);
    float dot01 = QVector3D::dotProduct(v0, v1);
    float dot02 = QVector3D::dotProduct(v0, v2);
    float dot11 = QVector3D::dotProduct(v1, v1);
    float dot12 = QVector3D::dotProduct(v1, v2);

    float d = (dot00 * dot11) - (dot01 * dot01);
    float u = ((dot11 * dot02) - (dot01*dot12))/d;
    float v = ((dot00 * dot12) - (dot01*dot02))/d;

    float w = 1.0f - u - v;

    return QVector3D(u, v, w);
}

QVector3D Mesh::getBarycentricCoordinates(QVector2D point, QVector2D A, QVector2D B, QVector2D C)
{
    float det = (B.y() - C.y())*(A.x() - C.x()) + (C.x() - B.x())*(A.y() - C.y());

    float uFactor = (point.x() - C.x())*(B.y() - C.y()) + (point.y() - C.y())*(C.x() - B.x());
    float vFactor = (point.x() - C.x())*(C.y() - A.y()) + (point.y() - C.y())*(A.x() - C.x());
    float wFactor = (point.x() - C.x())*(A.y() - B.y()) + (point.y() - C.y())*(A.x() - B.x());

    float u = uFactor / det;
    float v = vFactor / det;
    float w = 1 - u - v;

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
