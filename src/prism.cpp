#include "prism.h"


Prism::Prism(Triangle middle)
{
    m_middle = middle;

    QVector3D e1 = m_middle.B - m_middle.A;
    QVector3D e2 = m_middle.C - m_middle.A;
    m_normal = QVector3D::crossProduct(e1, e2);
    m_normal.normalize();

    m_inner.A = m_middle.A - (m_normal * (1.5f + 1.5f));
    m_inner.B = m_middle.B - (m_normal * (1.5f + 1.5f));
    m_inner.C = m_middle.C - (m_normal * (1.5f + 1.5f));

    m_outer.A = m_middle.A + (m_normal * 1.5f);
    m_outer.B = m_middle.B + (m_normal * 1.5f);
    m_outer.C = m_middle.C + (m_normal * 1.5f);

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

QVector3D Prism::checkWhere(QVector3D point)
{
    QVector3D closestPoint;

    float v = point.x();
    if(v < m_boundingBox.xMin)  { v = m_boundingBox.xMin; }
    if(v > m_boundingBox.xMax) { v = m_boundingBox.xMax; }

    closestPoint.setX(v);

    v = point.y();
    if(v < m_boundingBox.yMin)  { v = m_boundingBox.yMin; }
    if(v > m_boundingBox.yMax) { v = m_boundingBox.yMax; }

    closestPoint.setY(v);

    v = point.z();
    if(v < m_boundingBox.zMin)  { v = m_boundingBox.zMin; }
    if(v > m_boundingBox.zMax) { v = m_boundingBox.zMax; }

    closestPoint.setZ(v);

    return closestPoint;
}

BBox Prism::triangleBoundingBox(Triangle tri)
{
    BBox triBBox;

    for(uint i = 0; i < 3; ++i)
    {
        switch(i)
        {
        case 0:
            updateValues(tri.A, triBBox);
            break;

        case 1:
            updateValues(tri.B, triBBox);
            break;

        case 2:
            updateValues(tri.C, triBBox);
            break;
        }
    }

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
