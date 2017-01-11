#ifndef SETTINGS_H
#define SETTINGS_H

//This header is used to store structs which are used for setting various variables
//across several different algorithms and classes

struct MeshSettings
{
    bool hasDistanceField = false;
    bool hasSpherePack = false;
};

struct DistanceFieldSettings
{
    //The resolution of the distance field
    float xRes = 2.0f;
    float yRes = 2.0f;
    float zRes = 2.0f;

    //The outer and inner margins of the mesh
    float innerMargin = 1.0f;
    float outerMargin = 1.0f;

    //How far outside the mesh's AABB the
    //field should go
    float meshMargin = 2.0f;
};

struct SpherePackSettings
{
    float radius = 1.0f;
    float spacing = 1.75f;
    bool isMax = false;
    int maxNumber = 1000000;
    bool drawSpheres = true;
    float numSpheres= 0;

    //Whether the user wants to remove spheres
    //on the outside or inside of the mesh
    bool cullOuter = false;
    bool cullInner = false;
};

struct ConstraintSettings
{
    bool isMax = false;
    //Max number is set to something ridiculous just in case
    int maxNumber = 1000000;
    float constraintStrength = 0.5f;
    bool isDrawing = false;

    //Which type of constraint to use
    bool useFixed = false;
    bool useSpring = true;
};

struct ShaderSettings
{
    //Colour of the shader. Values will be 0 - 1 to help with OpenGL
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;

    bool wireframe = false;
    bool skinMesh = false;
    bool drawMesh = true;
};

#endif // SETTINGS_H
