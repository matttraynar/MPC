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

enum IntersectionType
{
    Vertex,
    Edge,
    Face
};

struct Triangle
{
    QVector3D A;
    QVector3D B;
    QVector3D C;
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

    //Setter methods
    void setWireMode();
    void setColour(QVector4D colour);

    //Get methods
    inline std::vector<QVector3D> getVerts() const { return m_verts; }
    inline QVector3D getSphereAt(int index) const  { return m_spherePositions[index];}
    inline int getSphereNum() const                { return m_spherePositions.size(); }

    //Colour of the mesh
    QVector4D m_colour;

private:
    void calculateMAABB(float &xMin, float &xMax,
                                    float &yMin, float &yMax,
                                    float &zMin, float &zMax);

    void createMAABB(QVector3D &xyz, QVector3D &Xyz, QVector3D &XyZ, QVector3D &xyZ,
                                QVector3D &xYz, QVector3D &XYz, QVector3D &XYZ, QVector3D &xYZ);

    bool pointInTriBBox(QVector3D p, Triangle t);

    int getIntersections(QVector3D p, QVector3D dir);

    void getPotentialTriangles(QVector3D point, std::vector<Intersections> &intersectionHolder);
    void getPotentialTriangles(QVector2D point, std::vector<Intersections> &intersectionHolder);

    float getXFromVec(QVector3D a, QVector3D b);

    QVector3D getBarycentricCoordinates(QVector3D point, QVector3D A, QVector3D B, QVector3D C);
    QVector3D getBarycentricCoordinates(QVector2D point, QVector2D A, QVector2D B, QVector2D C);

    QVector3D calculateTriNorm(Triangle tri);

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

    float m_radius;

    std::vector<QVector3D> m_pointPositions;
    std::vector< std::vector< std::vector< int> > > m_distancePoints;

    //Wireframe state
    bool m_wireframeMode;
};

#endif // MESH_H_
