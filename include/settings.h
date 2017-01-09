#ifndef SETTINGS_H
#define SETTINGS_H

struct MeshSettings
{
    bool hasDistanceField = false;
    bool hasSpherePack = false;
};

struct DistanceFieldSettings
{
    float xRes = 2.0f;
    float yRes = 2.0f;
    float zRes = 2.0f;

    float innerMargin = 1.0f;
    float outerMargin = 1.0f;
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
};

struct ConstraintSettings
{
    bool isMax = false;
    int maxNumber = 1000000;
    float constraintStrength = 0.5f;
    bool isDrawing = false;
};

struct ShaderSettings
{
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;

    bool wireframe = false;
    bool skinMesh = false;
    bool drawMesh = true;
};

#endif // SETTINGS_H
