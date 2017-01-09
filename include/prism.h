#ifndef PRISM_H
#define PRISM_H

#include <QVector3D>

#include <stdlib.h>

#include "triangle.h"

struct BBox
{
    float xMin = 1000000.0f;
    float xMax = -1000000.0f;

    float yMin = 1000000.0f;
    float yMax = -1000000.0f;

    float zMin = 1000000.0f;
    float zMax = -1000000.0f;

    void makeBBox(float x, float X,
                            float y, float Y,
                            float z, float Z)
    {
        xMin = x;
        xMax = X;
        yMin = y;
        yMax = Y;
        zMin = z;
        zMax = Z;
    }
};

class Prism
{
public:
    Prism(Triangle middle, float inner, float outer);
    ~Prism();

    bool bBoxContains(QVector3D point);
    QVector3D checkWhere(QVector3D point);

    inline BBox getBoundingBox() const { return m_boundingBox; }

    inline Triangle getTriangle() const { return m_middle; }

    inline float getXMin() const { return m_boundingBox.xMin; }
    inline float getXMax() const { return m_boundingBox.xMax; }

    inline float getYMin() const { return m_boundingBox.yMin; }
    inline float getYMax() const { return m_boundingBox.yMax; }

    inline float getZMin() const { return m_boundingBox.zMin; }
    inline float getZMax() const { return m_boundingBox.zMax; }

    inline QVector3D getNormal() const { return m_normal; }

private:
    BBox triangleBoundingBox(Triangle tri);

    void updateValues(QVector3D point, BBox &boundingBox);
    void updateBoundingBox(BBox boundingBox);

    Triangle m_inner;
    Triangle m_middle;
    Triangle m_outer;

    QVector3D m_normal;

    BBox m_boundingBox;
};

#endif // PRISM_H
