#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include "obj_loader.h"

using namespace std;

namespace obj_loader {

    Mesh load_obj(const char *path) {
        cout << "Loading OBJ file..." << endl;

        std::vector<Point3> vertices;
        std::vector<Vector3> normals;

        std::regex regex("^([0-9]+)(/([0-9]+)?(/([0-9]+)?)?)?$");

        ifstream file;
        file.open(path);
        if (!file.is_open()) {
            throw std::runtime_error(std::string("Failed to open file '") + path + "' for reading");
        }

        Mesh mesh = Mesh();

        string lineText;
        while (getline(file, lineText)) {
            istringstream line(lineText);
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
                // Vertex's normal
                Vector3 normal;
                line >> normal.x >> normal.y >> normal.z;
                normals.push_back(normal);

            } else if (op == "f") {
                // Face
                Face face;
                string vertexDefinition;
                while (line >> vertexDefinition) {
                    unsigned int vertexIndex = 0, textureIndex = 0, normalIndex = 0;
                    std::smatch matchResults;
                    std::regex_match(vertexDefinition, matchResults, regex);

                    FaceVertex faceVertex = {};

                    try {
                        vertexIndex = stoi(matchResults[1].str());
                        if (vertexIndex > 0) {
                            faceVertex.position = vertices[vertexIndex - 1];
                        }
                    } catch (invalid_argument &error) {
                        throw std::runtime_error(
                                std::string("Failed to parse OBJ file: invalid face vertex definition: ") +
                                error.what());
                    }

                    try {
                        textureIndex = stoi(matchResults[3].str());
                    } catch (invalid_argument &ignored) {
                        textureIndex = vertexIndex;
                    }

                    try {
                        normalIndex = stoi(matchResults[5].str());
                        if (normalIndex > 0) {
                            faceVertex.normal = normals[normalIndex - 1];
                        }
                    } catch (invalid_argument &ignored) {
                        if (normals.size() >= vertices.size()) {
                            std::cerr << "Falling back to normalIndex = vertexIndex" << std::endl;
                            faceVertex.normal = normals[vertexIndex - 1];
                        } else {
                            std::cerr << "Falling back to default normal vector" << std::endl;
                            Vector3 defaultNormal = Vector3();
                            defaultNormal.x = 0;
                            defaultNormal.y = 0;
                            defaultNormal.z = 1;
                            faceVertex.normal = defaultNormal;
                        }
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
