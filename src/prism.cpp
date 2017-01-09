#include "prism.h"


Prism::Prism(Triangle middle, float inner, float outer)
{
    m_middle = middle;

    QVector3D e1 = m_middle.B - m_middle.A;
    QVector3D e2 = m_middle.C - m_middle.A;
    m_normal = QVector3D::crossProduct(e1, e2);
    m_normal.normalize();

    m_inner.A = m_middle.A - (m_normal * inner);// * (1.5f + 1.5f));
    m_inner.B = m_middle.B - (m_normal * inner);// * (1.5f + 1.5f));
    m_inner.C = m_middle.C - (m_normal * inner);// * (1.5f + 1.5f));

    m_outer.A = m_middle.A + (m_normal * outer);// * 1.5f);
    m_outer.B = m_middle.B + (m_normal * outer);// * 1.5f);
    m_outer.C = m_middle.C + (m_normal * outer);// * 1.5f);

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
    QVector3D e1 = m_middle.B - m_middle.A;
    QVector3D e2 = m_middle.C - m_middle.A;
    QVector3D e3 = m_middle.C - m_middle.B;

    float sN = QVector3D::dotProduct(point - m_middle.A, e1);
    float sD = QVector3D::dotProduct(point - m_middle.B, m_middle.A - m_middle.B);

    float tN = QVector3D::dotProduct(point - m_middle.A, e2);
    float tD = QVector3D::dotProduct(point - m_middle.C, m_middle.A - m_middle.C);

    if(sN <= 0.0f && tN <= 0.0f)
    {
        return m_middle.A;
    }

    float uN = QVector3D::dotProduct(point - m_middle.B, e3);
    float uD = QVector3D::dotProduct(point - m_middle.C, m_middle.B - m_middle.C);

    if(sD <= 0.0f && uN <= 0.0f)
    {
        return m_middle.B;
    }

    if(tD <= 0.0f && uD <= 0.0f)
    {
        return m_middle.C;
    }

    QVector3D n = QVector3D::crossProduct(e1, e2);
    float vc = QVector3D::dotProduct(n, QVector3D::crossProduct(m_middle.A - point, m_middle.B - point));

    if(vc <= 0.0f && sN >= 0.0f && sD >= 0.0f)
    {
        return (m_middle.A + sN/(sN + sD) * e1);
    }

    float va = QVector3D::dotProduct(n, QVector3D::crossProduct(m_middle.B - point, m_middle.C - point));

    if(va <= 0.0f && uN >= 0.0f && uD >= 0.0f)
    {
        return(m_middle.B + uN/(uN + uD) * e3);
    }

    float vb = QVector3D::dotProduct(n, QVector3D::crossProduct(m_middle.C - point, m_middle.A - point));

    if(vb <= 0.0f && tN >= 0.0f && tD >= 0.0f)
    {
        return(m_middle.A + tN/(tN + tD) * e2);
    }

    float u = va/(va +vb + vc);
    float v = vb/(va + vb + vc);
    float w = 1.0f - u -v;

    return (u * m_middle.A) + (v * m_middle.B) + (w * m_middle.C);

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
