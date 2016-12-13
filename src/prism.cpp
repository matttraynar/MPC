#include "prism.h"


Prism::Prism(Triangle middle)
{
    m_middle = middle;

    QVector3D e1 = m_middle.B - m_middle.A;
    QVector3D e2 = m_middle.C - m_middle.A;
    m_normal = QVector3D::crossProduct(e1, e2);
    m_normal.normalize();

    m_inner.A = m_middle.A - (m_normal);// * (1.5f + 1.5f));
    m_inner.B = m_middle.B - (m_normal);// * (1.5f + 1.5f));
    m_inner.C = m_middle.C - (m_normal);// * (1.5f + 1.5f));

    m_outer.A = m_middle.A + (m_normal);// * 1.5f);
    m_outer.B = m_middle.B + (m_normal);// * 1.5f);
    m_outer.C = m_middle.C + (m_normal);// * 1.5f);

    BBox inBBox    = triangleBoundingBox(m_inner);
    BBox midBBox = triangleBoundingBox(m_middle);
    BBox outBBox  = triangleBoundingBox(m_outer);

    updateBoundingBox(inBBox);
    updateBoundingBox(midBBox);
    updateBoundingBox(outBBox);
}

Prism::~Prism()
{

}

bool Prism::bBoxContains(QVector3D point)
{
    if((point.x() >= m_boundingBox.xMin && point.x() <= m_boundingBox.xMax) &&
        (point.y() >= m_boundingBox.yMin && point.y() <= m_boundingBox.yMax) &&
        (point.z() >= m_boundingBox.zMin && point.z() <= m_boundingBox.zMax))
    {
        return true;
    }

    return false;
}

int compare(const void *a, const void *b)
{
    return ( *(float *)a - *(float *)b );
}

QVector3D Prism::checkWhere(QVector3D point)
{
    QVector3D closestPoint;

    float v = point.x();

    float xValues[] = { m_middle.A.x(), m_middle.B.x(), m_middle.C.x() };
    xValues[0];

    qsort(xValues, 3, sizeof(float), compare);

    if(v < xValues[0])
    {
        if(xValues[0] == m_middle.A.x())
        {
            v = m_middle.A.x();
        }
        else if(xValues[0] == m_middle.B.x())
        {
            v = m_middle.B.x();
        }
        else if(xValues[0] == m_middle.C.x())
        {
            v = m_middle.C.x();
        }
    }
    else if(v > xValues[2])
    {
        if(xValues[2] == m_middle.A.x())
        {
            v = m_middle.A.x();
        }
        else if(xValues[2] == m_middle.B.x())
        {
            v = m_middle.B.x();
        }
        else if(xValues[2] == m_middle.C.x())
        {
            v = m_middle.C.x();
        }
    }

    closestPoint.setX(v);

    v = point.y();

    float yValues[] = { m_middle.A.y(), m_middle.B.y(), m_middle.C.y() };
    qsort(yValues, 3, sizeof(float), compare);

    if(v < yValues[0])
    {
        if(yValues[0] == m_middle.A.y())
        {
            v = m_middle.A.y();
        }
        else if(yValues[0] == m_middle.B.y())
        {
            v = m_middle.B.y();
        }
        else if(yValues[0] == m_middle.C.y())
        {
            v = m_middle.C.y();
        }
    }
    else if(v > yValues[2])
    {
        if(yValues[2] == m_middle.A.y())
        {
            v = m_middle.A.y();
        }
        else if(yValues[2] == m_middle.B.y())
        {
            v = m_middle.B.y();
        }
        else if(yValues[2] == m_middle.C.y())
        {
            v = m_middle.C.y();
        }
    }

    closestPoint.setY(v);

    v = point.z();

    float zValues[] = { m_middle.A.z(), m_middle.B.z(), m_middle.C.z() };
    qsort(zValues, 3, sizeof(float), compare);

    if(v < zValues[0])
    {
        if(zValues[0] == m_middle.A.z())
        {
            v = m_middle.A.z();
        }
        else if(zValues[0] == m_middle.B.z())
        {
            v = m_middle.B.z();
        }
        else if(zValues[0] == m_middle.C.z())
        {
            v = m_middle.C.z();
        }
    }
    else if(v > zValues[2])
    {
        if(zValues[2] == m_middle.A.z())
        {
            v = m_middle.A.z();
        }
        else if(zValues[2] == m_middle.B.z())
        {
            v = m_middle.B.z();
        }
        else if(zValues[2] == m_middle.C.z())
        {
            v = m_middle.C.z();
        }
    }

    closestPoint.setZ(v);

    return closestPoint;
}

BBox Prism::triangleBoundingBox(Triangle tri)
{
    BBox triBBox;

    updateValues(tri.A, triBBox);
    updateValues(tri.B, triBBox);
    updateValues(tri.C, triBBox);

    return triBBox;
}

void Prism::updateValues(QVector3D point, BBox &boundingBox)
{
    if(point.x() < boundingBox.xMin)  { boundingBox.xMin = point.x(); }
    if(point.x() > boundingBox.xMax) { boundingBox.xMax = point.x(); }

    if(point.y() < boundingBox.yMin)  { boundingBox.yMin = point.y(); }
    if(point.y() > boundingBox.yMax) { boundingBox.yMax = point.y(); }

    if(point.z() < boundingBox.zMin)  { boundingBox.zMin = point.z(); }
    if(point.z() > boundingBox.zMax) { boundingBox.zMax = point.z(); }
}

void Prism::updateBoundingBox(BBox boundingBox)
{
    if(boundingBox.xMin < m_boundingBox.xMin)   { m_boundingBox.xMin = boundingBox.xMin; }
    if(boundingBox.xMax > m_boundingBox.xMax) { m_boundingBox.xMax = boundingBox.xMax; }

    if(boundingBox.yMin < m_boundingBox.yMin)   { m_boundingBox.yMin = boundingBox.yMin; }
    if(boundingBox.yMax > m_boundingBox.yMax) { m_boundingBox.yMax = boundingBox.yMax; }

    if(boundingBox.zMin < m_boundingBox.zMin)   { m_boundingBox.zMin = boundingBox.zMin; }
    if(boundingBox.zMax > m_boundingBox.zMax) { m_boundingBox.zMax = boundingBox.zMax; }
}
