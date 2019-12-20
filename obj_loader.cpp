#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include "obj_loader.h"

using namespace std;

namespace obj_loader {

    struct FaceVertexDefinition {
        size_t vertexOrdinal;
        size_t textureVertexOrdinal;
        size_t normalVectorOrdinal;
    };

    static const std::regex FACE_VERTEX_DEFINITION_PATTERN("^([0-9]+)(/([0-9]+)?(/([0-9]+)?)?)?$");

    FaceVertexDefinition parseVertexDefinition(string &definition) {
        FaceVertexDefinition result{
                .vertexOrdinal = 0,
                .textureVertexOrdinal = 0,
                .normalVectorOrdinal = 0,
        };

        std::smatch matchResults;
        std::regex_match(definition, matchResults, FACE_VERTEX_DEFINITION_PATTERN);

        try {
            result.vertexOrdinal = stoi(matchResults[1].str());
        } catch (invalid_argument &error) {
            throw std::runtime_error(std::string("Invalid face vertex index: '") + error.what() + "'");
        }

        try {
            result.textureVertexOrdinal = stoi(matchResults[3].str());
        } catch (invalid_argument &ignored) {
            result.textureVertexOrdinal = 0;
        }

        try {
            result.normalVectorOrdinal = stoi(matchResults[5].str());
        } catch (invalid_argument &ignored) {
            result.normalVectorOrdinal = 0;
        }

        return result;
    }

    Mesh load_obj(const char *path) {
        cout << "Loading OBJ file..." << endl;

        std::vector<Point3> vertices;
        std::vector<Point3> textureVertices;
        std::vector<Vector3> normals;

        ifstream file;
        file.open(path);
        if (!file.is_open()) {
            throw std::runtime_error(std::string("Failed to open file '") + path + "' for reading");
        }

        Mesh mesh = Mesh();

        string lineOfText;
        while (getline(file, lineOfText)) {
            istringstream line(lineOfText);
            std::string op;
            line >> op;

            if (op == "#" || op.empty()) {
                // Comment, ignore line

            } else if (op == "v") {
                // Vertex
                Point3 point;
                line >> point.x >> point.y >> point.z;
                vertices.push_back(point);

            } else if (op == "vn") {
                // Normal vector for a vertex
                Vector3 normal;
                line >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);

            } else if (op == "vt") {
                Point3 point;
                line >> point.x >> point.y >> point.z;
                textureVertices.push_back(point);

            } else if (op == "f") {
                // Face
                Face face;
                string vertexDefinition;
                bool shouldCalculateNormals = false;
                while (line >> vertexDefinition) {
                    FaceVertex faceVertex;
                    FaceVertexDefinition faceVertexDefinition = parseVertexDefinition(vertexDefinition);
                    faceVertex.position = vertices[faceVertexDefinition.vertexOrdinal - 1];

                    if (faceVertexDefinition.textureVertexOrdinal != 0) {
                        faceVertex.texture = textureVertices[faceVertexDefinition.textureVertexOrdinal - 1];
                    }

                    if (faceVertexDefinition.normalVectorOrdinal != 0) {
                        faceVertex.normal = normals[faceVertexDefinition.normalVectorOrdinal - 1];
                    } else {
                        shouldCalculateNormals = true;
                    }

                    face.vertices.push_back(faceVertex);
                    face.neigh.push_back(0);
                }

                if (shouldCalculateNormals) {
                    cerr << "Some normals not present for a face, calculating normal vectors" << endl;
                    Vector3 v12 = face.vertices[1].position - face.vertices[0].position;
                    Vector3 v13 = face.vertices[2].position - face.vertices[0].position;
                    const Vector3 &calculatedNormal = v12.cross_multiply(v13);

                    for (FaceVertex &vertex : face.vertices) {
                        vertex.normal = calculatedNormal;
                    }
                }

                mesh.faces.push_back(face);

            } else {
                cerr << "Unsupported operation '" << op << "'" << endl;
            }
        }

        file.close();

        return mesh;
    }
}

Vector3 Vector3::cross_multiply(Vector3 that) {
    return Vector3{
            .x = this->y * that.z - this->z * that.y,
            .y = that.x * this->z - that.z * this->x,
            .z = this->x * that.y - this->y * that.x,
    };
}

Vector3 Point3::operator-(Point3 that) {
    return Vector3{
            .x = this->x - that.x,
            .y = this->y - that.y,
            .z = this->z - that.z,
    };
}

bool Point3::operator==(const Point3 &rhs) const {
    return x == rhs.x &&
           y == rhs.y &&
           z == rhs.z;
}

bool Point3::operator!=(const Point3 &rhs) const {
    return !(rhs == *this);
}

PlaneEquation Face::calculatePlaneEquation() {
    PlaneEquation equation = {};
    Point3 v[4];
    int i;

    for (i = 0; i < 3; i++) {
        v[i + 1].x = this->vertices[i].position.x;
        v[i + 1].y = this->vertices[i].position.y;
        v[i + 1].z = this->vertices[i].position.z;
    }

    equation.a = v[1].y * (v[2].z - v[3].z) + v[2].y * (v[3].z - v[1].z) + v[3].y * (v[1].z - v[2].z);
    equation.b = v[1].z * (v[2].x - v[3].x) + v[2].z * (v[3].x - v[1].x) + v[3].z * (v[1].x - v[2].x);
    equation.c = v[1].x * (v[2].y - v[3].y) + v[2].x * (v[3].y - v[1].y) + v[3].x * (v[1].y - v[2].y);

    equation.d = -(v[1].x * (v[2].y * v[3].z - v[3].y * v[2].z) +
                   v[2].x * (v[3].y * v[1].z - v[1].y * v[3].z) +
                   v[3].x * (v[1].y * v[2].z - v[2].y * v[1].z));

    return equation;
}

Segment3::Segment3(const Point3 &pointA, const Point3 &pointB) : pointA(pointA), pointB(pointB) {}

bool Segment3::isEquivalentTo(Segment3 &that) {
    return (this->pointA == that.pointA && this->pointB == that.pointB) ||
           (this->pointA == that.pointB && this->pointB == that.pointA);
}
