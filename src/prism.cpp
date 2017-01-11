#include "prism.h"

Prism::Prism(Triangle middle, float inner, float outer)
{
    //Construct the prism
    m_middle = middle;

    QVector3D e1 = m_middle.B - m_middle.A;
    QVector3D e2 = m_middle.C - m_middle.A;
    m_normal = QVector3D::crossProduct(e1, e2);
    m_normal.normalize();

    //Scale the triangle inwards along the normal
    m_inner.A = m_middle.A - (m_normal * inner);
    m_inner.B = m_middle.B - (m_normal * inner);
    m_inner.C = m_middle.C - (m_normal * inner);

    //Scale the triangle outwards along the normal
    m_outer.A = m_middle.A + (m_normal * outer);
    m_outer.B = m_middle.B + (m_normal * outer);
    m_outer.C = m_middle.C + (m_normal * outer);

    //Calculate the bounding box of each triangle
    BBox inBBox    = triangleBoundingBox(m_inner);
    BBox midBBox = triangleBoundingBox(m_middle);
    BBox outBBox  = triangleBoundingBox(m_outer);

    //Update the prism's bounding box using these
    //smaller boxes
    updateBoundingBox(inBBox);
    updateBoundingBox(midBBox);
    updateBoundingBox(outBBox);
}

Prism::~Prism()
{

}

bool Prism::bBoxContains(QVector3D point)
{
    //Check if the point is within the prism's bounding box
    return  ((point.x() >= m_boundingBox.xMin && point.x() <= m_boundingBox.xMax) &&
                (point.y() >= m_boundingBox.yMin && point.y() <= m_boundingBox.yMax) &&
                (point.z() >= m_boundingBox.zMin && point.z() <= m_boundingBox.zMax));
}

QVector3D Prism::checkWhere(QVector3D point)
{
    //This is essentially the same as the barycentric interpolation done in the
    //getIntersection function of spherepack.cpp. However this is done to accertain
    //which voronoi region of the triangle the point lies in (vertex, edge, or face) and
    //subsequently where the closest point on the triangle to the given point is

    //Get the edges of the middle triangle
    QVector3D e1 = m_middle.B - m_middle.A;
    QVector3D e2 = m_middle.C - m_middle.A;
    QVector3D e3 = m_middle.C - m_middle.B;

    float sN = QVector3D::dotProduct(point - m_middle.A, e1);
    float sD = QVector3D::dotProduct(point - m_middle.B, m_middle.A - m_middle.B);

    float tN = QVector3D::dotProduct(point - m_middle.A, e2);
    float tD = QVector3D::dotProduct(point - m_middle.C, m_middle.A - m_middle.C);

    //This means the point lies within the vertex region close to vertex A.
    //Subsequently the closest point on the triangle *is* vertex A
    if(sN <= 0.0f && tN <= 0.0f)
    {
        return m_middle.A;
    }

    float uN = QVector3D::dotProduct(point - m_middle.B, e3);
    float uD = QVector3D::dotProduct(point - m_middle.C, m_middle.B - m_middle.C);

    //Similar case to before but we're now at vertex B
    if(sD <= 0.0f && uN <= 0.0f)
    {
        return m_middle.B;
    }

    //And this is the case fo vertex C
    if(tD <= 0.0f && uD <= 0.0f)
    {
        return m_middle.C;
    }

    //If we get to this point then we're not in a vertex region, so need
    //to test for edge or face intersection
    QVector3D n = QVector3D::crossProduct(e1, e2);
    float vc = QVector3D::dotProduct(n, QVector3D::crossProduct(m_middle.A - point, m_middle.B - point));

    //The point closest point lies somewhere in the AB edge region. Do a
    //linear interpolation to figure out where
    if(vc <= 0.0f && sN >= 0.0f && sD >= 0.0f)
    {
        return (m_middle.A + sN/(sN + sD) * e1);
    }

    float va = QVector3D::dotProduct(n, QVector3D::crossProduct(m_middle.B - point, m_middle.C - point));

    //Now in the BC edge region, return the closest point
    if(va <= 0.0f && uN >= 0.0f && uD >= 0.0f)
    {
        return(m_middle.B + uN/(uN + uD) * e3);
    }

    float vb = QVector3D::dotProduct(n, QVector3D::crossProduct(m_middle.C - point, m_middle.A - point));

    //Final edge region is the CA region so test that and return accordingly
    if(vb <= 0.0f && tN >= 0.0f && tD >= 0.0f)
    {
        return(m_middle.A + tN/(tN + tD) * e2);
    }

    //If all these tests have failed the point *must* be on the triangle somewhere
    //so it can easily be calculated using barycentric interpolation
    float u = va/(va +vb + vc);
    float v = vb/(va + vb + vc);
    float w = 1.0f - u -v;

    return (u * m_middle.A) + (v * m_middle.B) + (w * m_middle.C);

}

BBox Prism::triangleBoundingBox(Triangle tri)
{
    //Create a bounding box for the given triangle
    BBox triBBox;

    updateValues(tri.A, triBBox);
    updateValues(tri.B, triBBox);
    updateValues(tri.C, triBBox);

    return triBBox;
}

void Prism::updateValues(QVector3D point, BBox &boundingBox)
{
    //Update the values accordingly
    if(point.x() < boundingBox.xMin)  { boundingBox.xMin = point.x(); }
    if(point.x() > boundingBox.xMax) { boundingBox.xMax = point.x(); }

    if(point.y() < boundingBox.yMin)  { boundingBox.yMin = point.y(); }
    if(point.y() > boundingBox.yMax) { boundingBox.yMax = point.y(); }

    if(point.z() < boundingBox.zMin)  { boundingBox.zMin = point.z(); }
    if(point.z() > boundingBox.zMax) { boundingBox.zMax = point.z(); }
}

void Prism::updateBoundingBox(BBox boundingBox)
{
    //Pretty much the same as the previous function but tests data from bounding boxes
    //rather than a given point
    if(boundingBox.xMin < m_boundingBox.xMin)   { m_boundingBox.xMin = boundingBox.xMin; }
    if(boundingBox.xMax > m_boundingBox.xMax) { m_boundingBox.xMax = boundingBox.xMax; }

    if(boundingBox.yMin < m_boundingBox.yMin)   { m_boundingBox.yMin = boundingBox.yMin; }
    if(boundingBox.yMax > m_boundingBox.yMax) { m_boundingBox.yMax = boundingBox.yMax; }

    if(boundingBox.zMin < m_boundingBox.zMin)   { m_boundingBox.zMin = boundingBox.zMin; }
    if(boundingBox.zMax > m_boundingBox.zMax) { m_boundingBox.zMax = boundingBox.zMax; }
}
