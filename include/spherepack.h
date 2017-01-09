#ifndef SPHEREPACK_H_
#define SPHEREPACK_H_

#ifndef LARGE_PVE
#define LARGE_PVE 1000000000
#endif

#include <QVector3D>
#include <QDebug>
#include <QDialog>

#include <vector>
#include <memory>

#include "settings.h"
#include "prism.h"

typedef std::vector<QVector3D> vector_V;
typedef std::vector<int> int_V;
typedef std::vector<uint> uint_V;
typedef std::vector<Triangle> triangle_V;
typedef std::vector<Prism> prism_V;

typedef std::vector<QVector3D>::iterator vector_Iter;
typedef std::vector<int>::iterator int_Iter;
typedef std::vector<Prism>::iterator prism_Iter;

enum HaloIntersections
{
    NoHit,
    OneHit,
    TwoHits
};

/*-----------------------------
 * This struct uses a slightly modified version of code that can be found here:
 * http://stackoverflow.com/questions/33557276/using-find-with-vectorpairint-int
 *
 * The modification is to allow for reverse pairs e.g (A,B) and (B,A)
 * */
struct FindPair
{
    FindPair(int a, int b) : m_first(a), m_second(b) {}

    int m_first;
    int m_second;

    bool operator() (const std::pair<int, int> &pair)
    {
        return((pair.first == m_first && pair.second == m_second) ||
                 (pair.first == m_second && pair.second == m_first));
    }
};

class SpherePack
{
public:
    SpherePack();
    SpherePack(const vector_V &verts, const uint_V &indices, float radius);
    ~SpherePack();    

    void generateDistanceField(DistanceFieldSettings &settings);
    void generateDistanceField();
    void packSpheres(SpherePackSettings &settings);

    inline QVector3D getSphereAt(int index) const  { return m_spherePositions[index]; }
    inline uint getSphereNum() const                       { return (uint)m_spherePositions.size(); }
    inline vector_V getSpheres() const{ return m_spherePositions; }

    void getCloseSpheres(uint sphereIndex, vector_V &positions, std::vector<std::pair<uint, uint> > &pairs, int maxNumConstraints, float strength);

private:

    void findStartingPosition(uint triIndex, QVector3D &p1, QVector3D &p2);

    void packSpheres();

    void calculateMAABB(float margin);
    BBox makeNeighbourhood(const QVector3D &p);
    bool bBoxContains(const BBox &box, const QVector3D &point);

    int getIntersections(const QVector3D &p, const QVector3D &dir);
    void haloIntersection(const QVector3D &a, const QVector3D &b, const QVector3D &c,
                                    QVector3D &hit1, QVector3D &hit2,
                                    HaloIntersections &intersectionType);

    void validatePoints(vector_V &points);
    bool checkAgainstMesh(QVector3D point);
    void checkAgainstSpheres(QVector3D point, vector_V &points);

    QVector3D calculateTriNorm(Triangle tri);

    float interpolateLinear(const float &x, const float &x1, const float &x2, const float &c00, const float &c01);
    float interpolateTrilinear(const QVector3D &p);

    int getBestPoint(const QVector3D &currentSphere, vector_V points);

    float m_radius;
    float m_spacing;

    int m_sphereNum;

    QVector3D m_boxRes;
    float m_boxResolution;

    BBox m_meshAABB;

    vector_V m_verts;
    uint_V m_indices;
    vector_V m_spherePositions;

    //These two vectors are linked but I decided not to do these as a pair
    //as it would probably not be worth the hastle
    std::vector< std::vector< int_V > > m_distancePoints;
    std::vector< std::vector< triangle_V > > m_distanceTriangles;

    prism_V m_shell;
};

#endif // SPHEREPACK_H_
