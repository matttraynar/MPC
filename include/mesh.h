#ifndef MESH_H_
#define MESH_H_

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <QtGui>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QVector3D>

#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "prism.h"

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

enum IntersectionType
{
    Vertex,
    Edge,
    Face
};

enum HaloIntersections
{
    NoHit,
    OneHit,
    TwoHits
};

struct Ray
{
    QVector3D pos;
    QVector3D dir;
};

struct Intersections
{
    Triangle tri;
    QVector3D baryCentric;
    QVector3D pos;
    IntersectionType type;
};

class Mesh
{
public:
    //Ctors
    Mesh() = default;
    Mesh(QVector4D colour);

    //Dtor
    ~Mesh();

    //Methods for loading, passing to the shader and drawing the mesh
    void loadMesh(const char* filepath);
    void prepareMesh(QOpenGLShaderProgram &program);
    void draw();

    //Method that creates the point field
    void generateDistanceField();

    //Methods used to draw the point field of the mesh
    void preparePoints(QOpenGLShaderProgram &program);
    void drawPoints();


    //Method which creates a sphere pack for the mesh
    void packSpheres();

    //Set methods
    void setWireMode();
    void setColour(QVector4D colour);

    //Get methods
    inline std::vector<QVector3D> getVerts() const { return m_verts; }
    inline QVector3D getSphereAt(int index) const  { return m_spherePositions[index];}
    inline int getSphereNum() const                       { return (int)m_spherePositions.size(); }
    inline QVector4D getColour() const                  { return m_colour; }
    inline QVector3D getCOM() const                                { return m_COM; }

    void getCloseSpheres(uint sphereIndex, std::vector<QVector3D> &positions, std::vector<std::pair<uint, uint> > &pairs);

private:
    //Method which calculates the bounding box of the mesh
    void calculateMAABB();

    //Boolean function used to compare two triangles
    bool triangleEquals(Triangle a, Triangle b);

    //Function which gets the intersections between mesh faces
    //and a ray starting at p with direction dir
    int getIntersections(QVector3D p, QVector3D dir);

    //Method for calculating the normal of a triangle
    QVector3D calculateTriNorm(Triangle tri);

    //Method which creates a bounding box around a point
    BBox makeNeighbourhood(QVector3D p);

    //Method used to check whether a point lies within
    //the given bounding box or not
    bool bBoxContains(BBox box, QVector3D point);

    //Method used to check the intersections of 3 spheres using
    //halos (slightly extruded version of the spheres)
    void haloIntersection(QVector3D a, QVector3D b, QVector3D c, QVector3D &hit1, QVector3D &hit2, HaloIntersections &intersectionType);

    //Method used to check that a given set of sphere positions
    //lie within the mesh and do not cause collisions with other
    //spheres
    void validatePoints(std::vector<QVector3D> &points);

    //Two methods for interpolation
    float interpolateLinear(float x, float x1, float x2, float c00, float c01);
    float interpolateTrilinear(QVector3D p);

    //A method which returns the index of the point in a given container
    //which is the closest to the given point
    int getBestPoint(QVector3D currentSphere, std::vector<QVector3D> points);

    //Colour of the mesh
    QVector4D m_colour;

    //Array and buffer object for the mesh
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

    //Array and buffer object for the point field
    QOpenGLVertexArrayObject m_vaoP;
    QOpenGLBuffer m_vboP;

    //Containers for the mesh data
    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<uint> m_meshIndex;
    QVector3D m_COM;

    //Container with sphere positions
    std::vector<QVector3D> m_spherePositions;

    //The radius of spheres in the pack
    float m_radius;

    //The axis aligned bounding box of the mesh
    BBox m_meshAABB;

    //The resolution of the point field
    float m_boxResolution;

    //Containers for the point field, the signed distances of
    //those points and finally a container for the triangles
    //responsible for those distances
    std::vector<QVector3D> m_pointPositions;
    std::vector< std::vector< std::vector< int> > > m_distancePoints;
    std::vector< std::vector< std::vector<Triangle> > > m_distanceTriangles;

    //The shell of the mesh, made by turning each mesh
    //face into a prism
    std::vector<Prism> m_shell;

    //Wireframe state of the mesh
    bool m_wireframeMode;
};

#endif // MESH_H_
