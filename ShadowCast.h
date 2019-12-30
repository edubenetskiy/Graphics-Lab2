
#ifndef GRAPHICS_LAB2_SHADOWCAST_H
#define GRAPHICS_LAB2_SHADOWCAST_H

static const double SHADOW_INFINITY = 100;

#include <GL/gl.h>
#include <cstdlib>
#include "obj_loader.h"

// Calculate neighbors for a mesh's surfaces.
// This connectivity procedure is based on Gamasutra's article.
void calculateFaceAdjacency(Mesh &mesh);

void castShadow(Mesh &mesh, const double *lightPosition);

void castShadow(Mesh &mesh, const Vector3 &lightPosition);

bool gluInvertMatrix(const double m[16], double invOut[16]);

void multiplyMatVec(const double *mat, double *vec);

#endif //GRAPHICS_LAB2_SHADOWCAST_H
