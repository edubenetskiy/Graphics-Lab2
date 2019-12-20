
#include "obj_loader.h"
#include <GL/gl.h>
#include <cstdlib>
#include "ShadowCast.h"

// TODO: need to calculate neighbors
void castShadow(Mesh &mesh, double const *lightPosition) {
    unsigned int faceIndex, vertexIndex, neighborIndex, nextVertexIndex;
    FaceVertex faceVertex2;
    FaceVertex faceVertex1;
    Point3 v1, v2;

    //set visual parameter
    for (faceIndex = 0; faceIndex < mesh.faces.size(); faceIndex++) {
        PlaneEquation planeEquation = mesh.faces[faceIndex].calculatePlaneEquation();
        // check to see if light is in front or behind the plane (face plane)
        double side = planeEquation.a * lightPosition[0] +
                      planeEquation.b * lightPosition[1] +
                      planeEquation.c * lightPosition[2] +
                      planeEquation.d * lightPosition[3];
        mesh.faces[faceIndex].visible = side > 0;
    }

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_STENCIL_TEST);
    glColorMask(0, 0, 0, 0);
    glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

    // First pass, stencil operation decreases stencil value
    glFrontFace(GL_CCW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    for (faceIndex = 0; faceIndex < mesh.faces.size(); faceIndex++) {
        Face face = mesh.faces[faceIndex];
        if (face.visible) {
            for (vertexIndex = 0; vertexIndex < face.vertices.size(); vertexIndex++) {
                neighborIndex = face.neigh[vertexIndex];
                if (!(neighborIndex != 0 && mesh.faces[neighborIndex - 1].visible)) {
                    // here we have an edge, we must draw a polygon
                    faceVertex1 = face.vertices[vertexIndex];
                    nextVertexIndex = (vertexIndex + 1) % face.vertices.size();
                    faceVertex2 = face.vertices[nextVertexIndex];

                    //calculate the length of the vector
                    v1.x = (faceVertex1.position.x - lightPosition[0]) * SHADOW_INFINITY;
                    v1.y = (faceVertex1.position.y - lightPosition[1]) * SHADOW_INFINITY;
                    v1.z = (faceVertex1.position.z - lightPosition[2]) * SHADOW_INFINITY;

                    v2.x = (faceVertex2.position.x - lightPosition[0]) * SHADOW_INFINITY;
                    v2.y = (faceVertex2.position.y - lightPosition[1]) * SHADOW_INFINITY;
                    v2.z = (faceVertex2.position.z - lightPosition[2]) * SHADOW_INFINITY;

                    //draw the polygon
                    glBegin(GL_TRIANGLE_STRIP);
                    glVertex3f(faceVertex1.position.x,
                               faceVertex1.position.y,
                               faceVertex1.position.z);
                    glVertex3f(faceVertex1.position.x + v1.x,
                               faceVertex1.position.y + v1.y,
                               faceVertex1.position.z + v1.z);

                    glVertex3f(faceVertex2.position.x,
                               faceVertex2.position.y,
                               faceVertex2.position.z);
                    glVertex3f(faceVertex2.position.x + v2.x,
                               faceVertex2.position.y + v2.y,
                               faceVertex2.position.z + v2.z);
                    glEnd();
                }
            }
        }
    }

    // Second pass, stencil operation increases stencil value
    glFrontFace(GL_CW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    for (faceIndex = 0; faceIndex < mesh.faces.size(); faceIndex++) {
        Face face = mesh.faces[faceIndex];

        if (mesh.faces[faceIndex].visible)
            for (vertexIndex = 0; vertexIndex < mesh.faces[faceIndex].vertices.size(); vertexIndex++) {
                neighborIndex = face.neigh[vertexIndex];
                if ((!neighborIndex) || (!mesh.faces[neighborIndex - 1].visible)) {
                    // here we have an edge, we must draw a polygon
                    faceVertex1 = face.vertices[vertexIndex];
                    nextVertexIndex = (vertexIndex + 1) % mesh.faces[faceIndex].vertices.size();
                    faceVertex2 = face.vertices[nextVertexIndex];

                    //calculate the length of the vector
                    v1.x = (faceVertex1.position.x - lightPosition[0]) * SHADOW_INFINITY;
                    v1.y = (faceVertex1.position.y - lightPosition[1]) * SHADOW_INFINITY;
                    v1.z = (faceVertex1.position.z - lightPosition[2]) * SHADOW_INFINITY;

                    v2.x = (faceVertex2.position.x - lightPosition[0]) * SHADOW_INFINITY;
                    v2.y = (faceVertex2.position.y - lightPosition[1]) * SHADOW_INFINITY;
                    v2.z = (faceVertex2.position.z - lightPosition[2]) * SHADOW_INFINITY;

                    //draw the polygon
                    glBegin(GL_TRIANGLE_STRIP);
                    glVertex3f(faceVertex1.position.x,
                               faceVertex1.position.y,
                               faceVertex1.position.z);
                    glVertex3f(faceVertex1.position.x + v1.x,
                               faceVertex1.position.y + v1.y,
                               faceVertex1.position.z + v1.z);

                    glVertex3f(faceVertex2.position.x,
                               faceVertex2.position.y,
                               faceVertex2.position.z);
                    glVertex3f(faceVertex2.position.x + v2.x,
                               faceVertex2.position.y + v2.y,
                               faceVertex2.position.z + v2.z);
                    glEnd();
                }
            }
    }

    glFrontFace(GL_CCW);
    glColorMask(1, 1, 1, 1);

    // draw a shadowing rectangle covering the entire screen
    glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glStencilFunc(GL_NOTEQUAL, 0, 0xffffffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-0.1f, 0.1f, -0.10f);
    glVertex3f(-0.1f, -0.1f, -0.10f);
    glVertex3f(0.1f, 0.1f, -0.10f);
    glVertex3f(0.1f, -0.1f, -0.10f);
    glEnd();
    glPopMatrix();
    glDisable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glDisable(GL_STENCIL_TEST);
    glShadeModel(GL_SMOOTH);
}

void calculateFaceAdjacency(Mesh &mesh) {
    for (size_t faceIndexA = 0; faceIndexA < mesh.faces.size() - 1; faceIndexA++) {
        Face &faceA = mesh.faces[faceIndexA];

        for (size_t faceIndexB = faceIndexA + 1; faceIndexB < mesh.faces.size(); faceIndexB++) {
            Face &faceB = mesh.faces[faceIndexB];

            for (size_t currentVertexIndexA = 0; currentVertexIndexA < 3; currentVertexIndexA++) {
                size_t nextVertexIndexA = (currentVertexIndexA + 1) % faceA.vertices.size();

                Segment3 segmentA(faceA.vertices[currentVertexIndexA].position,
                                  faceA.vertices[nextVertexIndexA].position);

                if (!faceA.neigh[currentVertexIndexA]) {
                    for (size_t currentVertexIndexB = 0; currentVertexIndexB < 3; currentVertexIndexB++) {
                        size_t nextVertexIndexB = (currentVertexIndexB + 1) % faceB.vertices.size();
                        Segment3 segmentB(faceB.vertices[currentVertexIndexB].position,
                                          faceB.vertices[nextVertexIndexB].position);

                        if (segmentA.isEquivalentTo(segmentB)) {  // they are neighbours
                            faceA.neigh[currentVertexIndexA] = faceIndexB + 1;
                            faceB.neigh[currentVertexIndexB] = faceIndexA + 1;
                        }
                    }
                }
            }
        }
    }
}

bool gluInvertMatrix(const double *m, double *invOut) {
    double inv[16], det;
    int i;

    inv[0] = m[5] * m[10] * m[15] -
             m[5] * m[11] * m[14] -
             m[9] * m[6] * m[15] +
             m[9] * m[7] * m[14] +
             m[13] * m[6] * m[11] -
             m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
             m[4] * m[11] * m[14] +
             m[8] * m[6] * m[15] -
             m[8] * m[7] * m[14] -
             m[12] * m[6] * m[11] +
             m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
             m[4] * m[11] * m[13] -
             m[8] * m[5] * m[15] +
             m[8] * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
              m[4] * m[10] * m[13] +
              m[8] * m[5] * m[14] -
              m[8] * m[6] * m[13] -
              m[12] * m[5] * m[10] +
              m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
             m[1] * m[11] * m[14] +
             m[9] * m[2] * m[15] -
             m[9] * m[3] * m[14] -
             m[13] * m[2] * m[11] +
             m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
             m[0] * m[11] * m[14] -
             m[8] * m[2] * m[15] +
             m[8] * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
             m[0] * m[11] * m[13] +
             m[8] * m[1] * m[15] -
             m[8] * m[3] * m[13] -
             m[12] * m[1] * m[11] +
             m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
              m[0] * m[10] * m[13] -
              m[8] * m[1] * m[14] +
              m[8] * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
             m[1] * m[7] * m[14] -
             m[5] * m[2] * m[15] +
             m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
             m[0] * m[7] * m[14] +
             m[4] * m[2] * m[15] -
             m[4] * m[3] * m[14] -
             m[12] * m[2] * m[7] +
             m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
              m[0] * m[7] * m[13] -
              m[4] * m[1] * m[15] +
              m[4] * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
              m[0] * m[6] * m[13] +
              m[4] * m[1] * m[14] -
              m[4] * m[2] * m[13] -
              m[12] * m[1] * m[6] +
              m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
             m[1] * m[7] * m[10] +
             m[5] * m[2] * m[11] -
             m[5] * m[3] * m[10] -
             m[9] * m[2] * m[7] +
             m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
              m[0] * m[7] * m[9] +
              m[4] * m[1] * m[11] -
              m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] +
              m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

void multiplyMatVec(const double *mat, double *vec) {
    GLfloat result[4];

    result[0] = mat[0] * vec[0] + mat[4] * vec[1] + mat[8] * vec[2] + mat[12] * vec[3];
    result[1] = mat[1] * vec[0] + mat[5] * vec[1] + mat[9] * vec[2] + mat[13] * vec[3];
    result[2] = mat[2] * vec[0] + mat[6] * vec[1] + mat[10] * vec[2] + mat[14] * vec[3];
    result[3] = mat[3] * vec[0] + mat[7] * vec[1] + mat[11] * vec[2] + mat[15] * vec[3];

    vec[0] = result[0];
    vec[1] = result[1];
    vec[2] = result[2];
    vec[3] = result[3];
}
