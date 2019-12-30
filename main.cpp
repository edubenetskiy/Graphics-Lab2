#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <stack>
#include <queue>
#include "SOIL/SOIL.h"
#include "obj_loader.h"
#include "ShadowCast.h"

GLuint texture_wall;
GLuint texture_wood;

std::queue<char> keys = std::queue<char>();


float pyramid_rotation_angle;        // Angle For The Triangle
float cube_rotation_angle;    // Angle For The Quad
Mesh teapot;

enum class LightType {
    DIRECTED = 1,
    POINT = 2,
    PROJECTOR = 3,
    NO_LIGHT_SOURCES = 4,
    SUPPORT_DISABLED = 5,
};

LightType lightState = LightType::SUPPORT_DISABLED;

void drawWall();

void drawCube();

void drawPyramid();

void drawMesh(Mesh &mesh);

void placeAndRotateCamera();

void load_texture(const char *imageFilename, GLuint *textureId);

void printMatrix4x4(const double *modelViewMatrix);

void init() {
    glShadeModel(GL_SMOOTH);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);                                // The Type Of Depth Testing To Do
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClearDepth(1.0f);    // Depth Buffer Setup
    glClearStencil(0);                                    // Stencil Buffer Setup
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    // Really Nice Perspective Calculations

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    load_texture("textures/wall.jpg", &texture_wall);
    load_texture("textures/wood.png", &texture_wood);
    teapot = obj_loader::load_obj("meshes/heart.obj");
    calculateFaceAdjacency(teapot);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
}

void load_texture(const char *imageFilename, GLuint *textureId) {
    int width, height;
    unsigned char *image = SOIL_load_image(imageFilename, &width, &height, nullptr, SOIL_LOAD_RGB);
    if (image == nullptr) {
        std::cerr << "Failed to load texture from image file '" << imageFilename << "'." << std::endl;
        std::cerr << "The program should be started from the working directory that contains the image file."
                  << std::endl;
        exit(1);
    }

    glGenTextures(1, textureId);
    glBindTexture(GL_TEXTURE_2D, *textureId);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void reshape(int w, int h) {
    // Prevent division by zero, when window is too short
    // (you cant make a window of zero width).
    if (h == 0)
        h = 1;

    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set the clipping volume
    gluPerspective(55.0f, (GLfloat) w / (GLfloat) h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    placeAndRotateCamera();
}

double cameraRadius = 15.;
double cameraAngleY = 0.0;
double cameraAngleX = 0.0;
GLdouble camY = 0.;

/**
 * Устанавливает положение и угол обзора камеры.
 */
void placeAndRotateCamera() {
    GLdouble camX = sin(cameraAngleY) * cameraRadius;
    GLdouble camZ = cos(cameraAngleY) * cameraRadius;
    gluLookAt(camX, camY, camZ,
              0, 0, 0,
              0.0f, 1.0f, 0.0f);
}

void mainLoop() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (!keys.empty()) {
        char key = keys.back();
        switch (key) {
            case GLUT_KEY_LEFT:
                cameraAngleY -= 0.1;
                break;
            case GLUT_KEY_RIGHT:
                cameraAngleY += 0.1;
                break;
            case '+':
                cameraRadius -= 1.;
                break;
            case '-':
                cameraRadius += 1.;
                break;
            case 'r':
                cameraAngleX = 0.;
                cameraAngleY = 0.;
                camY = 0;
                cameraRadius = 15.;
                break;
            case 'w':
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                break;
            case 'v':
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                break;
            default:
                if (key == '~') lightState = LightType::SUPPORT_DISABLED;
                else if (key == '0') lightState = LightType::NO_LIGHT_SOURCES;
                else if (key == '1') lightState = LightType::DIRECTED;
                else if (key == '2') lightState = LightType::POINT;
                else if (key == '3') lightState = LightType::PROJECTOR;
                else std::cerr << "Unhandled key press: '" << keys.back() << "'!" << std::endl;
        }
        keys.pop();
    }

    GLfloat material_diffuse[] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

    double modelViewMatrix[16];
    double invertedModelViewMatrix[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);
    gluInvertMatrix(modelViewMatrix, invertedModelViewMatrix);
//        printMatrix4x4(modelViewMatrix);
    GLdouble lightPosition[] = {+0.0, 0.0, +15.0, 1.0};
    multiplyMatVec(invertedModelViewMatrix, lightPosition);

    glLoadIdentity();
    placeAndRotateCamera();

    if (lightState == LightType::DIRECTED) {
        // направленный источник света
        glEnable(GL_LIGHTING);
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        // автоматическое приведение нормалей к единичной длине
        glEnable(GL_NORMALIZE);
        GLfloat light0_diffuse_color[] = {0.4, 0.7, 0.2};
        GLfloat light0_direction[] = {0.0, 0.0, 1.0, 0.0};
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse_color);
        glLightfv(GL_LIGHT0, GL_POSITION, light0_direction);
    }
    if (lightState == LightType::POINT) {
        // точечный источник света
        // убывание интенсивности с расстоянием
        // отключено (по умолчанию)
        glEnable(GL_LIGHTING);
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        // автоматическое приведение нормалей к единичной длине
        glEnable(GL_NORMALIZE);
        GLfloat light2_diffuse[] = {1.0, 0.0, 0.0};
        GLfloat light2_position[] = {0.0, 3.0, 125.0, 1.0};
        glEnable(GL_LIGHT2);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_diffuse);
        glLightfv(GL_LIGHT2, GL_POSITION, light2_position);
        glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.);
        glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.);
        glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.);
    }
    if (lightState == LightType::PROJECTOR) {
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        GLfloat light3_diffuse[] = {0., 0., 1.};
        GLfloat light3_position[] = {+0.0, 0.0, -10.0, 1.0};
        GLfloat light3_spot_direction[] = {0.0, 0.0, -1.0};
        glEnable(GL_LIGHT3);
        glLightfv(GL_LIGHT3, GL_DIFFUSE, light3_diffuse);
        glLightfv(GL_LIGHT3, GL_POSITION, light3_position);
        glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 5);
        glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, light3_spot_direction);
    }
    if (lightState == LightType::NO_LIGHT_SOURCES) {
        // Все источники света отключены
        glEnable(GL_LIGHTING);
        glEnable(GL_NORMALIZE);
        glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT2);
        glDisable(GL_LIGHT3);
    }
    if (lightState == LightType::SUPPORT_DISABLED) {
        // Поддержка освещения отключена
        glDisable(GL_LIGHTING);
    }

    drawWall();
    drawPyramid();
    drawCube();

    glPushMatrix();
    {
        glTranslated(0., -2.5, +5.);
        glRotated(-5, 1., 1., 1.);
        glRotated(-90, 1., 0., 0.);

        drawMesh(teapot);
        castShadow(teapot, lightPosition);
    }
    glPopMatrix();

    pyramid_rotation_angle += 0.5f;
    cube_rotation_angle -= 0.15f;


    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);

    glutSwapBuffers();
}

void printMatrix4x4(const double *modelViewMatrix) {
    std::cout << std::endl << "----------------------------------" << std::endl;
    for (int i = 0; i < 16; ++i) {
        if (i % 4 == 0) {
            std::cout << std::endl;
        }
        std::cout << modelViewMatrix[i] << " ";
    }
}

GLfloat COLOR_RED[3] = {.67, .18, .15};
GLfloat COLOR_GREEN[3] = {.0, .59, .54};
GLfloat COLOR_BLUE[3] = {.02, .67, .96};

void drawMesh(Mesh &mesh) {
    double meshScale = 5.;
    glColor3fv(COLOR_RED);

    glBindTexture(GL_TEXTURE_2D, texture_wood);

    for (Face const &face: teapot.faces) {
        glBegin(GL_POLYGON);
        for (FaceVertex vertex : face.vertices) {
            glTexCoord3d(vertex.texture.x, vertex.texture.y, vertex.texture.z);
            glNormal3d(vertex.normal.x, vertex.normal.y, vertex.normal.z);
            glVertex3d(vertex.position.x / meshScale,
                       vertex.position.y / meshScale,
                       vertex.position.z / meshScale);
        }
        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
 * Рисует пирамидку.
 */
void drawPyramid() {
    glPushMatrix();

    glTranslatef(+2.f, 0.0f, 0.0f);
    glRotatef(pyramid_rotation_angle, 0.f, 1.0f, .0f);
    glBegin(GL_TRIANGLES);

    // Front face
    glColor3fv(COLOR_RED);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);            // Top Of Triangle (Front)
    glColor3fv(COLOR_GREEN);
    glVertex3f(-1.0f, -1.0f, 1.0f);            // Left Of Triangle (Front)
    glColor3fv(COLOR_BLUE);
    glVertex3f(1.0f, -1.0f, 1.0f);            // Right Of Triangle (Front)

    // Right face
    glColor3fv(COLOR_RED);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);            // Top Of Triangle (Right)
    glColor3fv(COLOR_BLUE);
    glVertex3f(1.0f, -1.0f, 1.0f);            // Left Of Triangle (Right)
    glColor3fv(COLOR_GREEN);
    glVertex3f(1.0f, -1.0f, -1.0f);            // Right Of Triangle (Right)

    // Back face
    glColor3fv(COLOR_RED);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);            // Top Of Triangle (Back)
    glColor3fv(COLOR_GREEN);
    glVertex3f(1.0f, -1.0f, -1.0f);            // Left Of Triangle (Back)
    glColor3fv(COLOR_BLUE);
    glVertex3f(-1.0f, -1.0f, -1.0f);            // Right Of Triangle (Back)

    // Left face
    glColor3fv(COLOR_RED);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);            // Top Of Triangle (Left)
    glColor3fv(COLOR_BLUE);
    glVertex3f(-1.0f, -1.0f, -1.0f);            // Left Of Triangle (Left)
    glColor3fv(COLOR_GREEN);
    glVertex3f(-1.0f, -1.0f, 1.0f);            // Right Of Triangle (Left)

    glEnd();                                            // Finished Drawing The Triangle

    // Bottom
    glBegin(GL_POLYGON);

    glColor3fv(COLOR_BLUE);
    glVertex3f(-1.f, -1.f, -1.f);

    glColor3fv(COLOR_GREEN);
    glVertex3f(+1.f, -1.f, -1.f);

    glColor3fv(COLOR_BLUE);
    glVertex3f(+1.f, -1.f, +1.f);

    glColor3fv(COLOR_GREEN);
    glVertex3f(-1.f, -1.f, +1.f);

    glEnd();

    glPopMatrix();
}

/**
 * Рисует куб.
 */
void drawCube() {
    glPushMatrix();
    glTranslatef(-2.f, 0.f, 0.f);                // Move Right 1.5 Units And Into The Screen 6.0
    glRotatef(cube_rotation_angle, 1.0f, 1.0f, 1.0f);

    glColor3f(1., 1., 1.);
    glBindTexture(GL_TEXTURE_2D, texture_wood);
    glBegin(GL_QUADS);

    // Front Face
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Back Face
    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    // Top Face
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    // Bottom Face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    // Right face
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Left Face
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    glEnd();            // Done Drawing The Quad
    glBindTexture(GL_TEXTURE_2D, 0);

    glPopMatrix();
}

/**
 * Рисует стену.
 */
void drawWall() {
    glPushMatrix();

    glTranslatef(0.0f, 0.0f, -3.0f);

    glColor3f(1., 1., 1.);
    glBindTexture(GL_TEXTURE_2D, texture_wall);

    int numFragments = 100;
    GLdouble width = 10.;
    GLdouble height = 8.;
    GLdouble left = -5.;
    GLdouble top = -4.;
    GLdouble fragmentWidth = width / numFragments;
    GLdouble fragmentHeight = height / numFragments;

    glBegin(GL_QUADS);
    for (int x = 0; x < numFragments; ++x) {
        for (int y = 0; y < numFragments; ++y) {
            GLdouble fragmentLeft = left + x * fragmentWidth;
            GLdouble fragmentTop = top + y * fragmentHeight;

            glNormal3f(0.0f, 0.0f, 1.0f);

            glTexCoord2d((double) (x) / numFragments, (double) (numFragments - y) / numFragments);
            glVertex3f(fragmentLeft, fragmentTop, 1.0f);  // Слева вверху

            glTexCoord2d((double) (x + 1) / numFragments, (double) (numFragments - y) / numFragments);
            glVertex3f(fragmentLeft + fragmentWidth, fragmentTop, 1.0f);  // Справа вверху

            glTexCoord2d((double) (x + 1) / numFragments, (double) (numFragments - y - 1) / numFragments);
            glVertex3f(fragmentLeft + fragmentWidth, fragmentTop + fragmentHeight, 1.0f);  // Справа внизу

            glTexCoord2d((double) (x) / numFragments, (double) (numFragments - y - 1) / numFragments);
            glVertex3f(fragmentLeft, fragmentTop + fragmentHeight, 1.0f);  // Слева внизу
        }
    }
    {
        glNormal3f(0.0f, 0.0f, -1.0f);

        glTexCoord2f(0.f, 0.f);
        glVertex3f(-5.0f, 4.0f, 1.0f);

        glTexCoord2f(1.f, 0.f);
        glVertex3f(5.0f, 4.0f, 1.0f);

        glTexCoord2f(1.f, 1.f);
        glVertex3f(5.0f, -4.0f, 1.0f);

        glTexCoord2f(0.f, 1.f);
        glVertex3f(-5.0f, -4.0f, 1.0f);
    }
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glPopMatrix();
}

/**
 * Обрабатывает нажатия клавиш клавиатуры.
 * @param key код клавиши
 * @param mouse_x координата X указателя мыши во время нажатия
 * @param mouse_y координата Y указателя мыши во время нажатия
 */
void keyboardHandler(unsigned char key, int mouse_x, int mouse_y) {
    std::cout << "Key '" << key << "' was pressed" << std::endl;
    keys.push(key);
    glutPostRedisplay();
}

void specialKeyHandler(int key, int x, int y) {
    std::cout << "Key '" << key << "' was pressed" << std::endl;

    switch (key) {
        case GLUT_KEY_UP:
            camY += 0.5;
            break;
        case GLUT_KEY_DOWN:
            camY -= 0.5;
            break;
        default:
            keys.push(key);
    }

    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL); // Display Mode
    glutInitWindowSize(1280, 800);
    glutCreateWindow("Laboratory work 2");
    init();

    glutDisplayFunc(mainLoop);  // Matching Earlier Functions To Their Counterparts
    glutReshapeFunc(reshape);
    glutIdleFunc(mainLoop);
    glutKeyboardFunc(keyboardHandler);
    glutSpecialFunc(specialKeyHandler);
    glutMainLoop();          // Initialize The Main Loop

    return 0;
}
