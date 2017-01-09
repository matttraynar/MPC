#include "spherepack.h"

SpherePack::SpherePack()
{
    m_radius = 1.0f;
    m_spacing = 1.9f;
}

SpherePack::SpherePack(const vector_V &verts, const uint_V &indices, float radius)
{
    //Create local copies of the data so we can use it later
    m_verts = verts;
    m_indices = indices;

    m_radius = 1.0f;
    m_spacing = 1.9f;
}

SpherePack::~SpherePack()
{

}

void SpherePack::generateDistanceField()
{
    //Intialise the bounding box minimum and maximum values
    //to large numbers to ensure no errors when calculating the
    //Margined Axis Aligned Bouding Box (MAABB)

    //Calculate the MAABB
    calculateMAABB(2.0f);

    m_boxResolution = 2.0f;

    float deltaPercent = (m_boxResolution/(m_meshAABB.yMax - m_meshAABB.yMin)) * 100;
    float totalPercent = 0.0f;

    Triangle defaultTri;

    qInfo() << "Creating distance point grid\n";

    //Iterate through every unit point in the MAABB, essentially creating a 3D grid
    for(float y = m_meshAABB.yMin; y <= m_meshAABB.yMax; y+=m_boxResolution)
    {
        std::vector< int_V > zDistances;
        std::vector< triangle_V > zTriangles;

        for(float z = m_meshAABB.zMin; z <= m_meshAABB.zMax; z+=m_boxResolution)
        {
            int_V xDistances;
            triangle_V xTriangles;

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
        if(totalPercent > 100.0f)
        {
            totalPercent = 100.0f;
        }
        qInfo() << "Distance points are "<<(int)totalPercent<<"% done\n";
    }

    qInfo() << "Initial distance point grid creation completed\n";

    for(uint i = 0; i < m_indices.size(); i += 3)
    {
        //Create the shells for the distance field. This is done
        //by taking each triangle face and extruding it into a prism
        //using the normal of the triangle face.
        Triangle t;
        t.A = m_verts[m_indices[i]];
        t.B = m_verts[m_indices[i + 1]];
        t.C = m_verts[m_indices[i + 2]];

        //The constructor generates the prism and a bounding box
        //for later
        Prism p(t, 1.0f, 1.0f);

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
                prism_Iter prism = m_shell.begin();

                while(prism != m_shell.end())
                {
                    QVector3D curPoint(x, y, z);

                    if(prism->bBoxContains(curPoint))
                    {
                        //Calcualte the closest point on the triangle face
                        //the prism was formed from to the grid point
                        QVector3D closestPoint = prism->checkWhere(curPoint);

                        //Then get the distance to that point
                        float distance = (closestPoint - curPoint).length();

                        //Check what the current value for 'closest distance'
                        //is
                        if(fabs(m_distancePoints[yI][zI][xI]) > fabs(distance))
                        {
                            //The distance is a closer one so we need to get the sign
                            //of the distance (simple dot product)
                            int sign = 0;
                            QVector3D::dotProduct(closestPoint, prism->getNormal()) < 0 ? sign = -1 : sign = 1;

                            //The distance is converted to a signed value
                            distance *= sign;

                            //A check is done to see if the currently stored triangle for this point
                            //is the default triangle inserted in the points generation stage
                            if((m_distanceTriangles[yI][zI][xI]) != defaultTri)
                            {
                                //The centers of the currently marked triangle and the potential new
                                //one are calculated
                                QVector3D aCenter = (m_distanceTriangles[yI][zI][xI].A +
                                                                m_distanceTriangles[yI][zI][xI].B +
                                                                m_distanceTriangles[yI][zI][xI].C) / 3.0f;

                                QVector3D bCenter = (prism->getTriangle().A +
                                                                prism->getTriangle().B +
                                                                prism->getTriangle().C) / 3.0f;

                                //The vector between the two centers is next calculated
                                QVector3D connectVector = bCenter - aCenter;

                                //By checking the dot of this connection vector with one of the triangle
                                //normals the check for concave/convex faces is made
                                float connectType = QVector3D::dotProduct(m_distanceTriangles[yI][zI][xI].getNormal(), connectVector);

                                if(connectType >= 0.0f)
                                {
                                    //Two faces are concave, make sure the signs are different
                                    //distance change is ONLY from negative to positive
                                    if((distance > 0.0f) && (m_distancePoints[yI][zI][xI] <= 0.0f))
                                    {
                                        //Set the new distance
                                        m_distancePoints[yI][zI][xI] = distance;

                                        //The triangle that forms the closest distance is also marked
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                                    }
                                    else if((distance <= 0.0f && m_distancePoints[yI][zI][xI] <= 0.0f) ||
                                             (distance > 0.0f && m_distancePoints[yI][zI][xI] > 0.0f))
                                    {
                                        //The distances have the same sign, set the new distance
                                        m_distancePoints[yI][zI][xI] = distance;

                                        //And mark the triangle
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
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
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                                    }
                                    else if((distance <= 0.0f && m_distancePoints[yI][zI][xI] <= 0.0f) ||
                                             (distance > 0.0f && m_distancePoints[yI][zI][xI] > 0.0f))
                                    {
                                        //Check again for distance with the same sign
                                        m_distancePoints[yI][zI][xI] = distance;
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                                    }
                                }

                            }
                            else
                            {
                                m_distancePoints[yI][zI][xI] = distance;

                                //The triangle that forms the closest distance is also marked
                                m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                            }
                        }
                    }

                    prism++;
                }
                x += m_boxResolution;

            }
            z += m_boxResolution;

        }
        y += m_boxResolution;
    }

    qInfo()<<"Shell creation finished";

}

void SpherePack::generateDistanceField(DistanceFieldSettings &settings)
{
    //Intialise the bounding box minimum and maximum values
    //to large numbers to ensure no errors when calculating the
    //Margined Axis Aligned Bouding Box (MAABB)

    //Calculate the MAABB
    calculateMAABB(settings.meshMargin);

//    m_boxResolution = 2.0f;

    float deltaPercent = (settings.yRes/(m_meshAABB.yMax - m_meshAABB.yMin)) * 100;
    float totalPercent = 0.0f;

    m_boxRes.setX(settings.xRes);
    m_boxRes.setY(settings.yRes);
    m_boxRes.setZ(settings.zRes);

    Triangle defaultTri;

    qInfo() << "Creating distance point grid\n";

    //Iterate through every unit point in the MAABB, essentially creating a 3D grid
    for(float y = m_meshAABB.yMin; y <= m_meshAABB.yMax; y+=settings.yRes)
    {
        std::vector< int_V > zDistances;
        std::vector< triangle_V > zTriangles;

        for(float z = m_meshAABB.zMin; z <= m_meshAABB.zMax; z+=settings.zRes)
        {
            int_V xDistances;
            triangle_V xTriangles;

            for(float x = m_meshAABB.xMin; x <= m_meshAABB.xMax; x+=settings.xRes)
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
        if(totalPercent > 100.0f)
        {
            totalPercent = 100.0f;
        }
        qInfo() << "Distance points are "<<(int)totalPercent<<"% done\n";
    }

    qInfo() << "Initial distance point grid creation completed\n";

    for(uint i = 0; i < m_indices.size(); i += 3)
    {
        //Create the shells for the distance field. This is done
        //by taking each triangle face and extruding it into a prism
        //using the normal of the triangle face.
        Triangle t;
        t.A = m_verts[m_indices[i]];
        t.B = m_verts[m_indices[i + 1]];
        t.C = m_verts[m_indices[i + 2]];

        //The constructor generates the prism and a bounding box
        //for later
        Prism p(t, settings.innerMargin, settings.outerMargin);

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
                prism_Iter prism = m_shell.begin();

                while(prism != m_shell.end())
                {
                    QVector3D curPoint(x, y, z);

                    if(prism->bBoxContains(curPoint))
                    {
                        //Calcualte the closest point on the triangle face
                        //the prism was formed from to the grid point
                        QVector3D closestPoint = prism->checkWhere(curPoint);

                        //Then get the distance to that point
                        float distance = (closestPoint - curPoint).length();

                        //Check what the current value for 'closest distance'
                        //is
                        if(fabs(m_distancePoints[yI][zI][xI]) > fabs(distance))
                        {
                            //The distance is a closer one so we need to get the sign
                            //of the distance (simple dot product)
                            int sign = 0;
                            QVector3D::dotProduct(closestPoint, prism->getNormal()) < 0 ? sign = -1 : sign = 1;

                            //The distance is converted to a signed value
                            distance *= sign;

                            //A check is done to see if the currently stored triangle for this point
                            //is the default triangle inserted in the points generation stage
                            if((m_distanceTriangles[yI][zI][xI]) != defaultTri)
                            {
                                //The centers of the currently marked triangle and the potential new
                                //one are calculated

                                //POSSIBLY CHANGE ---------------------------------------------------------
                                QVector3D aCenter = (m_distanceTriangles[yI][zI][xI].A +
                                                                m_distanceTriangles[yI][zI][xI].B +
                                                                m_distanceTriangles[yI][zI][xI].C) / 3.0f;

                                QVector3D bCenter = (prism->getTriangle().A +
                                                                prism->getTriangle().B +
                                                                prism->getTriangle().C) / 3.0f;

                                //The vector between the two centers is next calculated
                                QVector3D connectVector = bCenter - aCenter;

                                //By checking the dot of this connection vector with one of the triangle
                                //normals the check for concave/convex faces is made
                                float connectType = QVector3D::dotProduct(m_distanceTriangles[yI][zI][xI].getNormal(), connectVector);

                                if(connectType >= 0.0f)
                                {
                                    //Two faces are concave, make sure the signs are different
                                    //distance change is ONLY from negative to positive
                                    if((distance > 0.0f) && (m_distancePoints[yI][zI][xI] <= 0.0f))
                                    {
                                        //Set the new distance
                                        m_distancePoints[yI][zI][xI] = distance;

                                        //The triangle that forms the closest distance is also marked
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                                    }
                                    else if((distance <= 0.0f && m_distancePoints[yI][zI][xI] <= 0.0f) ||
                                             (distance > 0.0f && m_distancePoints[yI][zI][xI] > 0.0f))
                                    {
                                        //The distances have the same sign, set the new distance
                                        m_distancePoints[yI][zI][xI] = distance;

                                        //And mark the triangle
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
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
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                                    }
                                    else if((distance <= 0.0f && m_distancePoints[yI][zI][xI] <= 0.0f) ||
                                             (distance > 0.0f && m_distancePoints[yI][zI][xI] > 0.0f))
                                    {
                                        //Check again for distance with the same sign
                                        m_distancePoints[yI][zI][xI] = distance;
                                        m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                                    }
                                }

                            }
                            else
                            {
                                m_distancePoints[yI][zI][xI] = distance;

                                //The triangle that forms the closest distance is also marked
                                m_distanceTriangles[yI][zI][xI] = prism->getTriangle();
                            }
                        }
                    }

                    prism++;
                }
                x += m_boxResolution;

            }
            z += m_boxResolution;

        }
        y += m_boxResolution;
    }

    qInfo()<<"Shell creation finished";

}

void SpherePack::findStartingPosition(uint triIndex, QVector3D &p1, QVector3D &p2)
{
    //Make sure there aren't any spheres in the pack container
    m_spherePositions.clear();

    //Get one of the faces from the mesh
    Triangle startTri = m_shell[triIndex].getTriangle();

    //Get the normal and position of that face
    QVector3D triNorm = startTri.getNormal();
    QVector3D triMiddle = startTri.getMiddle();

    //Mark a point 1 radius way from this triangle
    //but in the -normal direction (so it is inside the mesh)
    QVector3D p0 = triMiddle - (triNorm * m_radius);

    m_spherePositions.push_back(p0);

    //Now create a second position
    p1 = p0 - (2 * triNorm * m_radius);

    //Ang finally a third
    p2 = p0 - (triNorm);
    QVector3D up = QVector3D::crossProduct(triNorm, QVector3D(1,0,0));
    p2 += up * sqrt(3.0) * m_radius;
}

void SpherePack::packSpheres()
{
    //Set some variables for later use
    QVector3D middlePos;

    //Calculate the middle of the mesh
    vector_Iter point = m_verts.begin();

    while(point != m_verts.end())
    {
        middlePos += *point;
        point++;
    }

    middlePos /= m_verts.size();

    //Create the start positions for the first 3 spheres
    QVector3D p0 = middlePos;
    QVector3D p1 = p0;
    QVector3D p2 = p0;

    p1[0] += 2 * m_radius;
    p2[0] += m_radius;

    //This value comes from looking at three circle
    //inscribed in an equilateral triangle.

    //E.G. : usercontent1.hubstatic.com/8432260_f520.jpg

    //The circle at the tip of the triangle will be √3 * radius
    //higher than the other two circle
    p2[1] += sqrt(3.0f) * m_radius;

    //Check each point we've created against the mesh
    if(!checkAgainstMesh(p0) || !checkAgainstMesh(p1) || !checkAgainstMesh(p2))
    {
        //One or more of the points was outside the mesh so
        //try finding a new set
        findStartingPosition(int(m_shell.size()) / 2, p1, p2);

        //Check again
        if(!checkAgainstMesh(p1) || !checkAgainstMesh(p2))
        {
            findStartingPosition(int(m_shell.size()) / 4, p1, p2);

            //Check once more
            if(!checkAgainstMesh(p1) || !checkAgainstMesh(p2))
            {
                findStartingPosition((int(m_shell.size()) / 4) * 3, p1, p2);

                if(!checkAgainstMesh(p1) || !checkAgainstMesh(p2))
                {
                    //Can't for certain say that no position for all three spheres exists
                    //without checking all possible add anything special to the program
                    //so just exit

                    qWarning()<<"Can't find a position, radius is probably too big";
                    exit(1);
                }
            }
        }
    }

    //At this point the other two positions should be within the mesh,
    //add them to our container
    m_spherePositions.push_back(p1);
    m_spherePositions.push_back(p2);

    //Add these intial positions to the sphere front
    std::vector<QVector3D> frontQueue;
    frontQueue.push_back(p1);
    frontQueue.push_back(p0);
    frontQueue.push_back(p2);

    qInfo()<<"Starting sphere pack";

    vector_V candidateStore;

    //Keep looping whilst there are still spheres in the front queue
    while(frontQueue.size() != 0)
    {
        qInfo()<<"Number of spheres: "<<m_spherePositions.size();

        //Get the position of the first 'sphere' in the queue
        QVector3D currentSphere = frontQueue[0];

        //First a check is done to see if there are any left over
        //candidates from previous iterations. This provides an
        //efficiency increase as these points don't have to be
        //calculated a second time
        if(candidateStore.size() > 0)
        {
            //This process is the same as the one done for normal
            //candidate points so for more info look further down
            //this function
            validatePoints(candidateStore);

            if(candidateStore.size() > 0)
            {
                int bestPoint = getBestPoint(currentSphere, candidateStore);

                m_spherePositions.push_back(candidateStore[bestPoint]);

                frontQueue.push_back(candidateStore[bestPoint]);
            }

            if(candidateStore.size() < 2)
            {
                //The only difference is ensuring the candidate store
                //is cleared to size = 0 when there are none left
                candidateStore.clear();
            }
        }
        else
        {
            //Calculate a region around it
            BBox neighbourhood = makeNeighbourhood(currentSphere);

            //Create a container for storing other spheres
            std::vector<QVector3D> neighbours;

            //Iterate through the queue (skipping the first
            //entry because that is the current sphere)
            for(uint i = 1; i < frontQueue.size(); ++i)
            {
                //Check if the region around the current sphere contains
                //the sphere at this position
                if(bBoxContains(neighbourhood, frontQueue[i]))
                {
                    //If it does then add it to our 'neighbourhood'
                    neighbours.push_back(frontQueue[i]);
                }
            }

            //Create a container to store candidate sphere
            //positions in
            std::vector<QVector3D> candidatePoints;

            //Set an iterator to the first element of the neighbourhood
            vector_Iter a = neighbours.begin();

            //Keep iterating until we reach the end
            while(a != neighbours.end())
            {
                //This is an NxN check so create a new iterator
                vector_Iter b = neighbours.begin();

                while(b != neighbours.end())
                {
                    //Skip when the iterators are equal
                    if(a == b)
                    {
                        b++;
                    }
                    else
                    {
                        //Create some data which will be overwritten by
                        //the next function call
                        QVector3D validPoint1;
                        QVector3D validPoint2;
                        HaloIntersections pointType;

                        //Check the current sphere for halo intersections with
                        //two other spheres in the neighbourhood
                        haloIntersection(currentSphere, *a, *b,
                                                validPoint1, validPoint2,
                                                pointType);

                        //Check what the result was
                        if(pointType != NoHit)
                        {
                            //There was at least one valid point so add that
                            //to the container
                            candidatePoints.push_back(validPoint1);

                            if(pointType == TwoHits)
                            {
                                //There were two valid points, add the second
                                candidatePoints.push_back(validPoint2);
                            }
                        }

                        //Increment the iterators
                        b++;
                    }
                }
                a++;
            }

            //Check all of the points we might be able to put a
            //sphere at to see if they are:
            // 1) Inside the mesh
            // 2) Not intersecting with any other spheres in the pack
            validatePoints(candidatePoints);

            //See if there are any candidate points left
            if(candidatePoints.size() > 0)
            {
                //If there are then just get the index of the closest point and
                //use that to add it to the sphere pack
                int bestPoint = getBestPoint(currentSphere, candidatePoints);

                m_spherePositions.push_back(candidatePoints[bestPoint]);

                //Also add this new sphere to the end of the front queue
                frontQueue.push_back(candidatePoints[bestPoint]);
            }

            //Finally check how many candidate points there are
            if(candidatePoints.size() < 2)
            {
                //In this case there was no more than 1 (which
                //has now been added to the pack). The current
                //has had the maximum number of spheres
                //packed around it so we can remove it and move
                //on to the next sphere in the front
                frontQueue.erase(frontQueue.begin());
            }
            else
            {
                //There are more candidates left. Store them so
                //they can be checked by the short circuit at the
                //start of this loop
                candidateStore = candidatePoints;
            }
        }
    }
}

void SpherePack::packSpheres(SpherePackSettings &settings)
{
   m_radius = settings.radius;
   m_spacing = settings.spacing;

    //Set some variables for later use
    QVector3D middlePos;

    //Calculate the middle of the mesh
    vector_Iter point = m_verts.begin();

    while(point != m_verts.end())
    {
        middlePos += *point;
        point++;
    }

    middlePos /= m_verts.size();

    //Create the start positions for the first 3 spheres
    QVector3D p0 = middlePos;
    QVector3D p1 = p0;
    QVector3D p2 = p0;

    p1[0] += 2 * m_radius;
    p2[0] += m_radius;

    //This value comes from looking at three circle
    //inscribed in an equilateral triangle.

    //E.G. : usercontent1.hubstatic.com/8432260_f520.jpg

    //The circle at the tip of the triangle will be √3 * radius
    //higher than the other two circle
    p2[1] += sqrt(3.0f) * m_radius;

    //Check each point we've created against the mesh
    if(!checkAgainstMesh(p0) || !checkAgainstMesh(p1) || !checkAgainstMesh(p2))
    {
        //One or more of the points was outside the mesh so
        //try finding a new set
        findStartingPosition(int(m_shell.size()) / 2, p1, p2);

        //Check again
        if(!checkAgainstMesh(p1) || !checkAgainstMesh(p2))
        {
            findStartingPosition(int(m_shell.size()) / 4, p1, p2);

            //Check once more
            if(!checkAgainstMesh(p1) || !checkAgainstMesh(p2))
            {
                findStartingPosition((int(m_shell.size()) / 4) * 3, p1, p2);

                if(!checkAgainstMesh(p1) || !checkAgainstMesh(p2))
                {
                    //Can't for certain say that no position for all three spheres exists
                    //without checking all possible add anything special to the program
                    //so just exit

                    qWarning()<<"Can't find a position, radius is probably too big";
                    exit(1);
                }
            }
        }
    }

    //At this point the other two positions should be within the mesh,
    //add them to our container
    m_spherePositions.push_back(p1);
    m_spherePositions.push_back(p2);

    //Add these intial positions to the sphere front
    std::vector<QVector3D> frontQueue;
    frontQueue.push_back(p1);
    frontQueue.push_back(p0);
    frontQueue.push_back(p2);

    qInfo()<<"Starting sphere pack";

    vector_V candidateStore;

    //Keep looping whilst there are still spheres in the front queue
    while(frontQueue.size() != 0)
    {
        qInfo()<<"Number of spheres: "<<m_spherePositions.size();

        if(settings.isMax)
        {
            if(m_spherePositions.size() > settings.maxNumber)
            {
                break;
            }
        }

        //Get the position of the first 'sphere' in the queue
        QVector3D currentSphere = frontQueue[0];

        //First a check is done to see if there are any left over
        //candidates from previous iterations. This provides an
        //efficiency increase as these points don't have to be
        //calculated a second time
        if(candidateStore.size() > 0)
        {
            //This process is the same as the one done for normal
            //candidate points so for more info look further down
            //this function
            validatePoints(candidateStore);

            if(candidateStore.size() > 0)
            {
                int bestPoint = getBestPoint(currentSphere, candidateStore);

                m_spherePositions.push_back(candidateStore[bestPoint]);

                frontQueue.push_back(candidateStore[bestPoint]);
            }

            if(candidateStore.size() < 2)
            {
                //The only difference is ensuring the candidate store
                //is cleared to size = 0 when there are none left
                candidateStore.clear();
            }
        }
        else
        {
            //Calculate a region around it
            BBox neighbourhood = makeNeighbourhood(currentSphere);

            //Create a container for storing other spheres
            std::vector<QVector3D> neighbours;

            //Iterate through the queue (skipping the first
            //entry because that is the current sphere)
            for(uint i = 1; i < frontQueue.size(); ++i)
            {
                //Check if the region around the current sphere contains
                //the sphere at this position
                if(bBoxContains(neighbourhood, frontQueue[i]))
                {
                    //If it does then add it to our 'neighbourhood'
                    neighbours.push_back(frontQueue[i]);
                }
            }

            //Create a container to store candidate sphere
            //positions in
            std::vector<QVector3D> candidatePoints;

            //Set an iterator to the first element of the neighbourhood
            vector_Iter a = neighbours.begin();

            //Keep iterating until we reach the end
            while(a != neighbours.end())
            {
                //This is an NxN check so create a new iterator
                vector_Iter b = neighbours.begin();

                while(b != neighbours.end())
                {
                    //Skip when the iterators are equal
                    if(a == b)
                    {
                        b++;
                    }
                    else
                    {
                        //Create some data which will be overwritten by
                        //the next function call
                        QVector3D validPoint1;
                        QVector3D validPoint2;
                        HaloIntersections pointType;

                        //Check the current sphere for halo intersections with
                        //two other spheres in the neighbourhood
                        haloIntersection(currentSphere, *a, *b,
                                                validPoint1, validPoint2,
                                                pointType);

                        //Check what the result was
                        if(pointType != NoHit)
                        {
                            //There was at least one valid point so add that
                            //to the container
                            candidatePoints.push_back(validPoint1);

                            if(pointType == TwoHits)
                            {
                                //There were two valid points, add the second
                                candidatePoints.push_back(validPoint2);
                            }
                        }

                        //Increment the iterators
                        b++;
                    }
                }
                a++;
            }

            //Check all of the points we might be able to put a
            //sphere at to see if they are:
            // 1) Inside the mesh
            // 2) Not intersecting with any other spheres in the pack
            validatePoints(candidatePoints);

            //See if there are any candidate points left
            if(candidatePoints.size() > 0)
            {
                //If there are then just get the index of the closest point and
                //use that to add it to the sphere pack
                int bestPoint = getBestPoint(currentSphere, candidatePoints);

                m_spherePositions.push_back(candidatePoints[bestPoint]);

                //Also add this new sphere to the end of the front queue
                frontQueue.push_back(candidatePoints[bestPoint]);
            }

            //Finally check how many candidate points there are
            if(candidatePoints.size() < 2)
            {
                //In this case there was no more than 1 (which
                //has now been added to the pack). The current
                //has had the maximum number of spheres
                //packed around it so we can remove it and move
                //on to the next sphere in the front
                frontQueue.erase(frontQueue.begin());
            }
            else
            {
                //There are more candidates left. Store them so
                //they can be checked by the short circuit at the
                //start of this loop
                candidateStore = candidatePoints;
            }
        }
    }
}

void SpherePack::calculateMAABB(float margin)
{
    //Set the intial values to amounts which
    //are guaranteed to be overwritten
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

    //Add a margin relative to the sphere radius to the bounding box

    xMin -=  m_radius * margin;
    xMax += m_radius * margin;

    yMin -= m_radius * margin;
    yMax += m_radius * margin;

    zMin -= m_radius * margin;
    zMax += m_radius * margin;

    //Store the values
    m_meshAABB.makeBBox(xMin, xMax,
                                            yMin, yMax,
                                            zMin, zMax);
}

BBox SpherePack::makeNeighbourhood(const QVector3D &p)
{
    //Create a new bounding box
    BBox nBBox;

    //Create a margin and set the values
    float margin = 3 * m_radius;
    nBBox.xMin = p.x() - margin;
    nBBox.xMax = p.x() + margin;

    nBBox.yMin = p.y() - margin;
    nBBox.yMax = p.y() + margin;

    nBBox.zMin = p.z() - margin;
    nBBox.zMax = p.z() + margin;

    return nBBox;
}

bool SpherePack::bBoxContains(const BBox &box, const QVector3D &point)
{
    //Simple boolean to check if a point is within certain margins
    return((point.x() >= box.xMin && point.x() <= box.xMax) &&
        (point.y() >= box.yMin && point.y() <= box.yMax) &&
        (point.z() >= box.zMin && point.z() <= box.zMax));
}

int SpherePack::getIntersections(const QVector3D &p, const QVector3D &dir)
{
    //Initialise the intersection counter and a boolean for later
    int hitCount = 0;
    bool isOnFace = false;

    //Initialise two new vectors for containing values when an
    //intersection occurs on a vertex or edge
    std::vector<float> vertHits;
    std::vector<float> edgeHits;

    //Iterate through each face in the mesh
    for(uint i = 0; i < m_indices.size(); i+=3)
    {
        //Get the vertex data from the face and add it to a triangle. The
        //mesh was triangulated upon load so faces with 3 vertices can be
        //assumed
        Triangle t;
        t.A = m_verts[m_indices[i]];
        t.B = m_verts[m_indices[i + 1]];
        t.C = m_verts[m_indices[i + 2]];

        //Calculate the vectors describing two of the face edges
        QVector3D e1 = t.B - t.A;
        QVector3D e2 = t.C - t.A;

        //By taking the cross product of these two edges a normal
        //for the face is obtained
        QVector3D N = QVector3D::crossProduct(e1, e2);

        //Take the cross between an edge and the ray direction
        QVector3D q = QVector3D::crossProduct(dir, e2);

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
        float v = QVector3D::dotProduct(r, dir) / a;

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

void SpherePack::haloIntersection(const QVector3D &a, const QVector3D &b, const QVector3D &c,
                                QVector3D &hit1, QVector3D &hit2,
                                HaloIntersections &intersectionType)
{
    //First get the distance between point A and B
    float dist = (a - b).length();

    //Set the radius of the halo to twice the normal radius
    float haloRadius = 2 * m_radius;

    //If the distance between A and B (plus the halo radius)
    //is smaller than the halo radius the two defintiely don't
    //intersect
    if((dist + haloRadius) < haloRadius)
    {
        intersectionType = NoHit;
        return;
    }

    //This value is taken from Eq.(10) of E. Lozano et al.'s paper.
    //They propose that
    //
    //alpha =  (1/2) * (1 - ((R2 ^2 - R1 ^2) / (|C1C2|^2))
    //
    //where R1 and R2 are the radii of two spheres and |C1C2| is
    //the distance between them. Because I am not using variable
    //radii this all cancels down to the below value
    float alpha = 0.5;

    //Get the center, normal and radius of the intersection circle
    //(when two spheres intersect you get a circle)
    QVector3D circleCenter = a + alpha*(b - a);

    float circleRadius = sqrt((haloRadius * haloRadius) - ((alpha * dist) * (alpha * dist)));

    QVector3D circleN = (a - b).normalized();

    //Get the smallest value of the circle normal. This is used later
    //when deciding on which world axis to use for calculating the
    //vector U
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

    //Here we have figured out whether the plane of the intersection circle
    //and the third point intersect or not
    if(fabs(dotDist) > haloRadius)
    {
        intersectionType = NoHit;
        return;
    }

    //The plane is at tangent to the third halo (so there is only one intersection)
    //We can stop here if so
    if(dotDist == haloRadius)
    {
        intersectionType = OneHit;
        return;
    }

    //Otherwise there is more than one intersection. Get the center and normal
    //of our second intersection circle
    QVector3D circleCenter2 = c + (dotDist * circleN);
    QVector3D circleN2 = QVector3D::crossProduct(circleCenter2 - circleCenter, circleN).normalized();

    //Mark the number of hits (circle + circle intersection  = two points)
    intersectionType = TwoHits;

   //And finally calculate the points
    hit1 = circleCenter - (circleN2 * circleRadius);
    hit2 = circleCenter + (circleN2 * circleRadius);
}


void SpherePack::validatePoints(vector_V &points)
{
    //Create a vector for storing valid points in
    vector_V acceptedPoints;
    vector_Iter point = points.begin();

    while(point != points.end())
    {
        //Check all of the points against the mesh
        if(checkAgainstMesh(*point))
        {
            //Add any points which are inside the mesh
            //to the container
            acceptedPoints.push_back(*point);
        }

        point++;
    }

    //Copy the points across
    points = acceptedPoints;

    //Clear the container because we're going to
    //use it again
    acceptedPoints.clear();

    //A container for marking point indices
    int_V pointsToRemove;

    //Store the number of points so we don't
    //have to call size() more than once
    uint numPoints = (uint)points.size();

    //Iterate through all the spheres added so far
    for(uint i = 0; i <m_spherePositions.size(); ++i)
    {
        //Then iterate through the sphere points the function was given
        for(uint j = 0; j < numPoints; ++j)
        {
            //See if the current point has already been marked for removal or not
            if(std::find(pointsToRemove.begin(), pointsToRemove.end(), j) == pointsToRemove.end())
            {
                //If it hasn't get the distance between the current sphere
                //and the current sphere in the pack
                float dist = (m_spherePositions[i] - points[j]).length();

                //Check if the distance means the two spheres
                //are colliding. Not using value slightly lower
                //than 2 * radius just to catch errors
                if(dist <= m_radius * m_spacing)
                {
                    pointsToRemove.push_back(j);
                }
            }
        }
    }

    //Now we need to go through the points and remove the ones which will
    //cause overlaps
    for(uint i = 0; i < numPoints; ++i)
    {
        //This is actually done by only adding points to our 'accepted' container
        //which were not marked for removal. This way we don't alter the size of
        //the original container and mess things up
        if(std::find(pointsToRemove.begin(), pointsToRemove.end(), i) == pointsToRemove.end())
        {
            acceptedPoints.push_back(points[i]);
        }
    }

    //Finally copy the points again
    points = acceptedPoints;
}

bool SpherePack::checkAgainstMesh(QVector3D point)
{
    //Get the signed distance of the point relative to the mesh
    float distance = interpolateTrilinear(point);

    //If the signed distance is less than the radius it can be said
    //that it is inside the mesh. Otherwise it is not
    return(distance <= m_radius);
}

float SpherePack::interpolateLinear(const float &x, const float &x1, const float &x2,
                                                    const float &c00, const float &c01)
{
    //Simply return the linear interpolation of the value (would be c02)
    return ( ((x2 - x) / (x2 - x1)) * c00 ) + ( ((x - x1) / (x2 - x1)) * c01 );
}

float SpherePack::interpolateTrilinear(const QVector3D &p)
{
    //Get the size of the y distance points
    int ySize = (int)m_distancePoints.size();
    float y =m_meshAABB.yMin;

    //Run through the distance field
    for(int yIndex = 0; yIndex < ySize - 1; yIndex++)
    {
        int zSize = (int)m_distancePoints[yIndex].size();

        float z = m_meshAABB.zMin;

        for(int zIndex = 0; zIndex < zSize - 1; zIndex++)
        {
            int xSize = (int)m_distancePoints[yIndex][zIndex].size();

            float x = m_meshAABB.xMin;

            //Now at the xIndex so first do a check to see if the point lies within
            //the current voxel
            for(int xIndex = 0; xIndex < xSize - 1; xIndex++)
            {
                if((p.x() >= x && p.x() <= (x + m_boxRes.x())) &&
                    (p.y() >= y && p.y() <= (y + m_boxRes.y())) &&
                    (p.z() >= z && p.z() <= (z + m_boxRes.z())) )
                {
                    //If it does do trilinear interpolation to get the signed distance
                    //of the point
                    float x00 = interpolateLinear(p.x(), x, (x + m_boxRes.x()),
                                                            m_distancePoints[yIndex][zIndex][xIndex],
                                                            m_distancePoints[yIndex][zIndex][xIndex + 1]);

                    float x10 = interpolateLinear(p.x(), x, (x + m_boxRes.x()),
                                                            m_distancePoints[yIndex + 1][zIndex][xIndex],
                                                            m_distancePoints[yIndex + 1][zIndex][xIndex + 1]);

                    float x01 = interpolateLinear(p.x(), x, (x + m_boxRes.x()),
                                                            m_distancePoints[yIndex][zIndex + 1][xIndex],
                                                            m_distancePoints[yIndex][zIndex + 1][xIndex + 1]);

                    float x11 = interpolateLinear(p.x(), x, (x + m_boxRes.x()),
                                                            m_distancePoints[yIndex + 1][zIndex + 1][xIndex],
                                                            m_distancePoints[yIndex + 1][zIndex + 1][xIndex + 1]);

                    float y0 = interpolateLinear(p.y(), y, (y + m_boxRes.y()),
                                                             x00,
                                                             x01);

                    float y1 = interpolateLinear(p.y(), y, (y + m_boxRes.y()),
                                                             x10,
                                                             x11);

                    float value = interpolateLinear(p.z(), z, (z + m_boxRes.z()),
                                                                y0,
                                                                y1);
                    return value;
                }

                x += m_boxRes.x();
            }

            z += m_boxRes.z();
        }

        y += m_boxRes.y();
    }

    //The point wasn't inside any of the voxels so return a stupidly big number.
    return 100000000.0f;
}

int SpherePack::getBestPoint(const QVector3D &currentSphere, vector_V points)
{
    //Pretty simply function which gets the index of the closest point
    //to the current sphere
    float distance = LARGE_PVE;
    int index = 0;

    for(uint i = 0; i < points.size(); ++i)
    {
        //Get the distance
        float tmpDistance = (points[i] - currentSphere).length();

        if(tmpDistance < distance)
        {
            //If the new distance is smaller than the old
            //shortest distance then change it and mark
            //the index
            distance = tmpDistance;
            index = i;
        }
    }

    //Return the index of the closest point
    return index;
}

void SpherePack::getCloseSpheres(uint sphereIndex, vector_V &positions, std::vector<std::pair<uint, uint> > &pairs, int maxNumConstraints)
{
    //Get the position of the current sphere 'locally' so we don't have to
    //keep indexing into the vector
    QVector3D curSphere = m_spherePositions[sphereIndex];

    //Set a margin for the points
    float margin = 10 * m_radius;

    //Iterate through all the spheres
    for(uint i = 0; i < m_spherePositions.size(); ++i)
    {
        //Ignore the sphere we're testing
        if(i == sphereIndex)
        {
            continue;
        }

        //The constraint pairs are stored in the passed in container.
        //If the size of that container is bigger than the desired
        //max number of constraints per sphere then exit
        if(pairs.size() > maxNumConstraints)
        {
           qInfo()<<"Constraint limit reached";
           break;
        }

        //Get a sphere from the list of positions
        QVector3D testSphere = m_spherePositions[i];

        //Calculate a region on either side of the x value
        float xMin = curSphere.x() - margin;
        float xMax = curSphere.x() + margin;        

        //Do the same for the y component
        float yMin = curSphere.y() - margin;
        float yMax = curSphere.y() + margin;

        //And finally the z component
        float zMin = curSphere.z() - margin;
        float zMax = curSphere.z() + margin;

        //Put this data into a voxel around the point
        BBox voxel;
        voxel.makeBBox(xMin, xMax,
                                yMin, yMax,
                                zMin, zMax);

        //Check if the sphere being tested is within the current
        //sphere's voxel. This means we don't have to do the
        //length for each point (essentially removing a large
        //amount of computationally heavy square roots)
        if(bBoxContains(voxel, testSphere))
        {
            //It is so get the distance between the two
            float distance = (testSphere - curSphere).length();

            //Check if the distance is acceptable
            if(distance <= margin)
            {
                //See if the constraint pair already exists
                auto pairPosition = std::find_if(pairs.begin(), pairs.end(), FindPair(sphereIndex, i));

                //This indicates the pair was not found
                if(pairPosition == pairs.end())
                {
                    //Add it to the list of accepted constraint positions
                    positions.push_back(testSphere);
                    pairs.push_back(std::make_pair(sphereIndex, i));
                }
            }
        }
    }
}
