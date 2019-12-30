
#include "obj_loader.h"
#include <GL/gl.h>
#include <cstdlib>
#include <GL/glu.h>
#include "ShadowCast.h"

void shadowPass(const Mesh &mesh, const Vector3 &lightPosition);

void castShadow(Mesh &mesh, double const *lightPosition) {
    Vector3 position = {.x=lightPosition[0], .y=lightPosition[1], .z=lightPosition[2]};
    castShadow(mesh, position);
}

void castShadow(Mesh &mesh, const Vector3 &lightPosition) {
    {
        // Draw light source as a sphere
        GLUquadric *quadric = gluNewQuadric();
        glPushMatrix();
        glTranslated(lightPosition.x, lightPosition.y, lightPosition.z);
        gluSphere(quadric, .25, 20, 20);
        glPopMatrix();
    }

    //set visual parameter
    for (Face &face : mesh.faces) {
        PlaneEquation planeEquation = face.calculatePlaneEquation();
        // check to see if light is in front or behind the plane (face plane)
        double side = planeEquation.a * lightPosition.x +
                      planeEquation.b * lightPosition.y +
                      planeEquation.c * lightPosition.z +
                      planeEquation.d; // TODO: Should be multiplied by lightPosition.w, which is normally 1.0
        face.visible = side > 0;
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
    shadowPass(mesh, lightPosition);

    // Second pass, stencil operation increases stencil value
    glFrontFace(GL_CW);
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    shadowPass(mesh, lightPosition);

    glFrontFace(GL_CCW);
    glColorMask(1, 1, 1, 1);

    // draw a shadowing rectangle covering the entire screen
    glColor4f(0.f, 0.f, 0.f, 0.5f);
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

void shadowPass(const Mesh &mesh, const Vector3 &lightPosition) {
    for (const Face &face : mesh.faces) {
        if (face.visible) {
            for (size_t vertexIndex = 0; vertexIndex < face.vertices.size(); vertexIndex++) {
                size_t neighborIndex = face.neigh[vertexIndex];
                if (!neighborIndex || !(mesh.faces[neighborIndex - 1].visible)) {
                    // here we have an edge, we must draw a polygon
                    FaceVertex faceVertex1 = face.vertices[vertexIndex];
                    size_t nextVertexIndex = (vertexIndex + 1) % face.vertices.size();
                    FaceVertex faceVertex2 = face.vertices[nextVertexIndex];

                    //calculate the length of the vector
                    Vector3 v1 = (faceVertex1.position - lightPosition) * SHADOW_INFINITY;
                    Vector3 v2 = (faceVertex2.position - lightPosition) * SHADOW_INFINITY;

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
