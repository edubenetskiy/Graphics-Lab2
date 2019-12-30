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

    Vector3 operator-(Vector3 that);

    Vector3 operator*(double that);

    bool operator==(const Vector3 &rhs) const;

    bool operator!=(const Vector3 &rhs) const;
};

struct Segment3 {
    Vector3 pointA;
    Vector3 pointB;

    Segment3(const Vector3 &pointA, const Vector3 &pointB);

public:
    bool isEquivalentTo(Segment3 &that);
};

struct PlaneEquation {
    double a = 0.;
    double b = 0.;
    double c = 0.;
    double d = 0.;
};

static const Vector3 DEFAULT_TEXTURE_VERTEX = {0., 0., 0.};
static const Vector3 DEFAULT_NORMAL_VECTOR = {0., 0., 1.};

class FaceVertex {
public:
    Vector3 position;
    Vector3 normal = DEFAULT_NORMAL_VECTOR;
    Vector3 texture = DEFAULT_TEXTURE_VERTEX;
};

class Face {
public:
    std::vector<FaceVertex> vertices;
    std::vector<size_t> neigh;
    bool visible;

    PlaneEquation calculatePlaneEquation();
};

class Mesh {
public:
    std::vector<Face> faces = std::vector<Face>();
};

namespace obj_loader {
    Mesh load_obj(const char *path, double scale);
}

#endif //GRAPHICS_LAB2_OBJ_LOADER_H
