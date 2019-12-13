//
// Created by egor on 12/12/19.
//

#ifndef GRAPHICS_LAB2_OBJ_LOADER_H
#define GRAPHICS_LAB2_OBJ_LOADER_H

#include <vector>

struct Vector3 {
    double x = 0.;
    double y = 0.;
    double z = 0.;

public:
    Vector3 cross_multiply(Vector3 that);
};

struct Point3 {
    double x = 0.;
    double y = 0.;
    double z = 0.;

public:
    Vector3 operator-(Point3 that);
};

static const Point3 DEFAULT_TEXTURE_VERTEX = {0., 0., 0.};
static const Vector3 DEFAULT_NORMAL_VECTOR = {0., 0., 1.};

class FaceVertex {
public:
    Point3 position;
    Vector3 normal = DEFAULT_NORMAL_VECTOR;
    Point3 texture = DEFAULT_TEXTURE_VERTEX;
};

class Face {
public:
    std::vector<FaceVertex> vertices;
};

class Mesh {
public:
    std::vector<Face> faces = std::vector<Face>();
};

namespace obj_loader {
    Mesh load_obj(const char *path);
}

#endif //GRAPHICS_LAB2_OBJ_LOADER_H
