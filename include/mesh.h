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

    void preparePoints(QOpenGLShaderProgram &program);
    void drawPoints();

    void generateDistanceField();

    //Method which creates a sphere pack for the mesh
    void packSpheres();
    void packSpheres2D();

    void getCloseSpheres(uint sphereIndex, std::vector<QVector3D> &positions, std::vector<std::pair<uint, uint> > &pairs);

    //Setter methods
    void setWireMode();
    void setColour(QVector4D colour);

    //Get methods
    inline std::vector<QVector3D> getVerts() const { return m_verts; }
    inline QVector3D getSphereAt(int index) const  { return m_spherePositions[index];}
    inline int getSphereNum() const                { return (int)m_spherePositions.size(); }

    inline std::vector<int> getConnectionsTo(int index) { return m_connections[index]; }

    //Colour of the mesh
    QVector4D m_colour;

private:
    void calculateMAABB();

    bool pointInTriBBox(QVector3D p, Triangle t);
    bool triangleEquals(Triangle a, Triangle b);

    int getIntersections(QVector3D p, QVector3D dir);

    void getPotentialTriangles(QVector3D point, std::vector<Intersections> &intersectionHolder);
    void getPotentialTriangles(QVector2D point, std::vector<Intersections> &intersectionHolder);

    float getXFromVec(QVector3D a, QVector3D b);

    QVector3D getBarycentricCoordinates(QVector3D point, QVector3D A, QVector3D B, QVector3D C);
    QVector3D getBarycentricCoordinates(QVector2D point, QVector2D A, QVector2D B, QVector2D C);

    QVector3D calculateTriNorm(Triangle tri);

    BBox makeNeighbourhood(QVector3D p);
    bool bBoxContains(BBox box, QVector3D point);
    void haloIntersection(QVector3D a, QVector3D b, QVector3D c, QVector3D &hit1, QVector3D &hit2, HaloIntersections &intersectionType);
    void validatePoints(std::vector<QVector3D> &points);

    float interpolateLinear(float x, float x1, float x2, float c00, float c01);
    float interpolateTrilinear(QVector3D p);

    int getBestPoint(QVector3D currentSphere, std::vector<QVector3D> points);

    //Array and buffer object for the mesh
    QOpenGLVertexArrayObject m_vao;
    QOpenGLVertexArrayObject m_vaoP;
    QOpenGLBuffer m_vboP;
    QOpenGLBuffer m_vbo;
    QOpenGLBuffer m_nbo;
    QOpenGLBuffer m_ibo;

    //Containers for the mesh data
    std::vector<QVector3D> m_verts;
    std::vector<QVector3D> m_norms;
    std::vector<uint> m_meshIndex;

    //Container with sphere positions
    std::vector<QVector3D> m_spherePositions;
    std::vector<QVector3D> m_activeSpheres;

    float m_radius;

    BBox m_meshAABB;
    float m_boxResolution;

    std::vector<QVector3D> m_pointPositions;
    std::vector< std::vector< std::vector< int> > > m_distancePoints;
    std::vector< std::vector< std::vector<Triangle> > > m_distanceTriangles;

    std::vector<Prism> m_shell;

    std::unordered_map<int, std::vector<int> > m_connections;

    //Wireframe state
    bool m_wireframeMode;
};

#endif // MESH_H_
