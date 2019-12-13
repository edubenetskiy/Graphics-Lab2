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

            } else if (op == "f") {
                // Face
                Face face;
                string vertexDefinition;
                while (line >> vertexDefinition) {
                    FaceVertex faceVertex;
                    FaceVertexDefinition faceVertexDefinition = parseVertexDefinition(vertexDefinition);
                    faceVertex.position = vertices[faceVertexDefinition.vertexOrdinal - 1];

                    if (faceVertexDefinition.normalVectorOrdinal != 0) {
                        faceVertex.normal = normals[faceVertexDefinition.normalVectorOrdinal - 1];
                    } else {
                        std::cerr << "Falling back to default normal vector" << std::endl;
                        faceVertex.normal = DEFAULT_NORMAL_VECTOR;
                    }

                    face.vertices.push_back(faceVertex);
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
