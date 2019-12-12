//
// Created by egor on 12/12/19.
//

#ifndef GRAPHICS_LAB2_OBJ_LOADER_H
#define GRAPHICS_LAB2_OBJ_LOADER_H

#include <vector>

class Point3 {
public:
    double x = 0.;
    double y = 0.;
    double z = 0.;

    Point3() = default;
};

class Vector3 {
public:
    double x = 0.;
    double y = 0.;
    double z = 0.;

    Vector3() = default;
};

class FaceVertex {
public:
    Point3 position;
    Vector3 normal;
};

class Face {
public:
    std::vector<FaceVertex> vertices = std::vector<FaceVertex>();
};

class Mesh {
public:
    std::vector<Face> faces = std::vector<Face>();
};

namespace obj_loader {
    Mesh load_obj(const char *path);
}

#endif //GRAPHICS_LAB2_OBJ_LOADER_H
