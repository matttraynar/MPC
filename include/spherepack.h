#ifndef SPHEREPACK_H_
#define SPHEREPACK_H_

#ifndef LARGE_PVE
#define LARGE_PVE 1000000000
#endif

#include <QVector3D>
#include <QDebug>

#include <vector>
#include <memory>

#include "settings.h"
#include "prism.h"

//Typedef a couple vectors so that if I need to change their
//data type later I can
typedef std::vector<QVector3D> vector_V;
typedef std::vector<int> int_V;
typedef std::vector<uint> uint_V;
typedef std::vector<Triangle> triangle_V;
typedef std::vector<Prism> prism_V;

//Do the same with iterators
typedef std::vector<QVector3D>::iterator vector_Iter;
typedef std::vector<int>::iterator int_Iter;
typedef std::vector<Prism>::iterator prism_Iter;

//An enum used in the halo intersections function
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
    //Ctor and dtors
    SpherePack();
    SpherePack(const vector_V &verts, const uint_V &indices, float radius);
    ~SpherePack();    

    //The two key functions, one generates a signed distance field,
    //the other runs the sphere packing algorithm
    void generateDistanceField(const DistanceFieldSettings &settings);
    void packSpheres(const SpherePackSettings &settings);

    //A few getters
    inline QVector3D getSphereAt(int index) const  { return m_spherePositions[index]; }
    inline uint getSphereNum() const                       { return (uint)m_spherePositions.size(); }
    inline vector_V getSpheres() const{ return m_spherePositions; }

    //Slightly different get, is used when creating constraints to return
    //the spheres which are in 'range' of the current sphere (marked with
    //the index)
    void getCloseSpheres(uint sphereIndex, vector_V &positions, std::vector<std::pair<uint, uint> > &pairs, int maxNumConstraints, float strength);

private:
    //Function used when the center of mass is not inside the mesh (I.E. cannot
    //be used as a start point for the sphere packing algorithm
    void findStartingPosition(uint triIndex, QVector3D &p1, QVector3D &p2);

    //Function which calculates the axis-aligned bounding box of a mesh
    //with a given margin added
    void calculateMAABB(float margin);

    //Used to create a voxel around a given point
    BBox makeNeighbourhood(const QVector3D &p);

    //Used to check if a bounding box contains a certain point
    bool bBoxContains(const BBox &box, const QVector3D &point);

    //Gets the intersection between a ray starting at p with direction 'dir'
    //and the mesh. Used in signed distance calculation
    int getIntersections(const QVector3D &p, const QVector3D &dir);

    //Takes three points and checks to see if three halos centred
    //at these points intersect
    void haloIntersection(const QVector3D &a, const QVector3D &b, const QVector3D &c,
                                    QVector3D &hit1, QVector3D &hit2,
                                    HaloIntersections &intersectionType);

    //A function which takes in a container of points and checks to see if
    //they are 1) within the mesh & 2) not intersecting any other points
    //where spheres have already been placed
    void validatePoints(vector_V &points);

    //Function which performs a check to see if a point is within the mesh
    bool checkAgainstMesh(QVector3D point);

    //Two functions used for interpolation. One is a linear interpolation (2D space)
    //the other does a trilinear interpolation (3D space)
    float interpolateLinear(const float &x, const float &x1, const float &x2, const float &c00, const float &c01);
    float interpolateTrilinear(const QVector3D &p);

    //Used to find the closest point from a given container to a given point.
    //Returns the index of that point
    int getBestPoint(const QVector3D &currentSphere, vector_V points);

    //The sphere radius being used
    float m_radius;

    //How spaced the spheres should be
    float m_spacing;

    //The resolution of the signed distance field cube. Stored in a vector for ease
    QVector3D m_boxRes;

    //The axis aligned bounding box of the mesh
    BBox m_meshAABB;

    //Containers for the vertices of the mesh, the face indices and finally one
    //for storing sphere positions in
    vector_V m_verts;
    uint_V m_indices;
    vector_V m_spherePositions;

    //These two vectors are linked but I decided not to do these as a pair
    //as it would probably not be worth the hastle
    std::vector< std::vector< int_V > > m_distancePoints;
    std::vector< std::vector< triangle_V > > m_distanceTriangles;

    //A container which essentially contains three versions of the mesh:
    //one which is scaled inwards, the original mesh and one which is scaled
    //outwards. Used in distance field calculation
    prism_V m_shell;
};

#endif // SPHEREPACK_H_
