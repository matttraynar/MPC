#ifndef SPHEREPACK_H__
#define SPHEREPACK_H__

#include <QVector3D>

#include <vector>

#include "prism.h"

enum HaloIntersections
{
    NoHit,
    OneHit,
    TwoHits
};

typedef std::vector<QVector3D> vector_V;
typedef std::vector<int> int_V;
typedef std::vector<Triangle> triangle_V;
typedef std::vector<Prism> prism_V;

typedef std::vector<QVector3D>::iterator vector_Iter;


class SpherePack : public Mesh
{
public:
    SpherePack();
    SpherePack(const vector_V &verts);

    inline QVector3D getSphereAt(int index) const  { return m_spherePositions[index];}
    inline int getSphereNum() const                       { return (int)m_spherePositions.size(); }

    void getCloseSpheres(uint sphereIndex, vector_V &positions,
                                    std::vector<std::pair<uint, uint> > &pairs);

private:
    void generateDistanceField();
    void packSpheres();

    void calculateMAABB();
    BBox makeNeighbourhood(const QVector3D &p);
    bool bBoxContains(BBox box, const QVector3D &point);

    int getIntersections(const QVector3D &p, const QVector3D &dir);
    void haloIntersection(const QVector3D &a, const QVector3D &b, const QVector3D &c,
                                    QVector3D &hit1, QVector3D &hit2,
                                    HaloIntersections &intersectionType);

    void validatePoints(V_Vector &points);

    float interpolateLinear(float x, float x1, float x2, float c00, float c01);
    float interpolateTrilinear(QVector3D p);

    int getBestPoint(const QVector3D &currentSphere, vector_V points);

    vector_V m_verts;
    vector_V m_spherePositions;

    vector_V m_pointPositions;
    std::vector< std::vector< int_V > > m_distancePoints;
    std::vector< std::vector< triangle_V > > m_distanceTriangles;

    prism_V m_shell;
};

#endif // SPHEREPACK_H
