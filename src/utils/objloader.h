#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <glm/glm.hpp>
#include <GL/glew.h>

struct FaceData {
    int defLineNum;                 // line number of face definition
    std::string defLine;            // line contents of face definition
    std::vector<int> vertInds;      // indices of the positiosn of each vertex
    bool defTexture;                // whether or not the face has defined texture indices
    std::vector<int> textureInds;   // indices of the texture coordinates of each vertex
    bool defNormals;                // whether or not the face has defined normal indices
    std::vector<int> normalInds;    // indices of the normals of each vertex
};

class OBJLoader {
public:
    // Parses and returns vertex data contained in the given object file
    // (Returned data only includes vertex positions and normals)
    static std::vector<GLfloat> parseObjFile(std::string objFilePath) {
        std::vector<GLfloat> vertexData;

        std::ifstream file(objFilePath);
        if (!file.is_open()) {
            std::cerr << "Unable to open file" << std::endl;
            return vertexData;
        }

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<FaceData> faces;

        std::string line;
        int lineNum = 1;
        while (std::getline(file, line)) {
            std::vector<std::string> tokens = tokenize(line, " \t\n\r");
            if(tokens.size() == 0) { // blank line
                lineNum++;
                continue;
            }
            if(tokens[0] == "v") {
                // vertex definition
                if(tokens.size() < 4 || tokens.size() > 5) {
                    std::cerr << "Error parsing directive in " << objFilePath << " on line " << lineNum << ": " << line << std::endl;
                    std::cerr << "Expected directive of the format: v x y z [w]" << std::endl;
                    return vertexData;
                }
                glm::vec3 vertex(1);
                for(int i = 0; i < 3; i++) {
                    if(!parseAsFloat(tokens[i+1], &vertex[i])) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                        std::cerr << "Could not parse " << tokens[i+1] << " as a float" << std::endl;
                        return vertexData;
                    }
                }
                vertices.push_back(vertex);
            } else if(tokens[0] == "vt") {
                // texture coordinate definition
                if(tokens.size() < 2 || tokens.size() > 4) {
                    std::cerr << "Error parsing directive in " << objFilePath << " on line " << lineNum << ": " << line << std::endl;
                    std::cerr << "Expected directive of the format: vt u [v w]" << std::endl;
                    return vertexData;
                }
                glm::vec3 uvCoords(0);
                for(int i = 0; i < tokens.size() - 1; i++) {
                    if(!parseAsFloat(tokens[i+1], &uvCoords[i])) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                        std::cerr << "Could not parse " << tokens[i+1] << " as a float" << std::endl;
                        return vertexData;
                    }
                }
                // texture coordinates aren't used
            } else if(tokens[0] == "vn") {
                // texture normal definition
                if(tokens.size() != 4) {
                    std::cerr << "Error parsing directive in " << objFilePath << " on line " << lineNum << ": " << line << std::endl;
                    std::cerr << "Expected directive of the format: vn x y z" << std::endl;
                    return vertexData;
                }
                glm::vec3 normal(0);
                for(int i = 0; i < 3; i++) {
                    if(!parseAsFloat(tokens[i+1], &normal[i])) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                        std::cerr << "Could not parse " << tokens[i+1] << " as a float" << std::endl;
                        return vertexData;
                    }
                }
                normals.push_back(normal);
            } else if(tokens[0] == "vp") {
                std::cerr << "Error parsing directive in " << objFilePath << " on line " << lineNum << ": " << line << std::endl;
                std::cerr << "Free-form geometry statements are unsupported" << std::endl;
                return vertexData;
            } else if(tokens[0] == "f") {
                // face definition
                if(tokens.size() != 4) {
                    std::cerr << "Error parsing directive in " << objFilePath << " on line " << lineNum << ": " << line << std::endl;
                    std::cerr << "Only triangle faces are supported" << std::endl;
                    return vertexData;
                }
                FaceData faceData = {
                    .defLineNum = lineNum,
                    .defLine = line,
                    .vertInds = std::vector<int>(3),
                    .defTexture = false,
                    .textureInds = std::vector<int>(3),
                    .defNormals = false,
                    .normalInds = std::vector<int>(3)
                };
                for(int i = 1; i < 4; i++) {
                    std::vector<std::string> indTokens = tokenize(tokens[i], "/");
                    if(indTokens.size() == 0 || indTokens.size() > 3) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                        std::cerr << "Invalid vertex specifier, expected <vertex index>[/<texture index>/<normal index>]" << std::endl;
                        return vertexData;
                    }

                    // Parse vertex index
                    if(!parseAsInt(indTokens[0], &faceData.vertInds[i-1])) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                        std::cerr << "Could not parse " << indTokens[0] << " as an integer index" << std::endl;
                        return vertexData;
                    }

                    // Parse optional texture index
                    if(indTokens.size() >= 2 && indTokens[1] != "") {
                        if(!parseAsInt(indTokens[1], &faceData.textureInds[i-1])) {
                            std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                            std::cerr << "Could not parse " << indTokens[1] << " as an integer index" << std::endl;
                            return vertexData;
                        }
                        faceData.defTexture = true;
                    } else if(faceData.defTexture) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                        std::cerr << "Not all vertices have a defined texture index" << std::endl;
                        return vertexData;
                    }

                    // Parse optional normal index
                    if(indTokens.size() >= 3 && indTokens[2] != "") {
                        if(!parseAsInt(indTokens[2], &faceData.normalInds[i-1])) {
                            std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                            std::cerr << "Could not parse " << indTokens[2] << " as an integer index" << std::endl;
                            return vertexData;
                        }
                        faceData.defNormals = true;
                    } else if(faceData.defNormals) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << i << ": " << line << std::endl;
                        std::cerr << "Not all vertices have a defined normal index" << std::endl;
                    }
                }
                faces.push_back(faceData);
            } else if(tokens[0] == "l") {
                std::cerr << "Error parsing directive in " << objFilePath << " on line " << lineNum << ": " << line << std::endl;
                std::cerr << "Line geometry statements are unsupported" << std::endl;
                return vertexData;
            } else if(tokens[0][0] != '#') {
                std::cerr << "Unknown directive in " << objFilePath << " on line " << lineNum << ": " << line << std::endl;
                return vertexData;
            }
            lineNum++;
        }
        file.close();

        // Parse face data into vertex data array
        for(FaceData& faceData : faces) {
            // Ensure all vertices are defined
            for(int i = 0; i < 3; i++) {
                int vertexInd = faceData.vertInds[i];
                if(vertexInd < 1 || vertexInd > vertices.size()) {
                    std::cerr << "Error parsing directive in " << objFilePath << " on line " << faceData.defLineNum << ": " << faceData.defLine << std::endl;
                    std::cerr << "Invalid vertex index " << vertexInd << " (" << vertices.size() << " vertices defined)" << std::endl;
                    return std::vector<GLfloat>();
                }
            }

            glm::vec3 calcNormal;
            if(!faceData.defNormals) {
                glm::vec3 edge1 = vertices[faceData.vertInds[2] - 1] - vertices[faceData.vertInds[1] - 1];
                glm::vec3 edge2 = vertices[faceData.vertInds[0] - 1] - vertices[faceData.vertInds[1] - 1];
                calcNormal = glm::cross(edge1, edge2);
            }
            for(int i = 0; i < 3; i++) {
                vertexData.push_back(vertices[faceData.vertInds[i] - 1].x);
                vertexData.push_back(vertices[faceData.vertInds[i] - 1].y);
                vertexData.push_back(vertices[faceData.vertInds[i] - 1].z);

                if(faceData.defNormals) {
                    int normalInd = faceData.normalInds[i];
                    if(normalInd < 1 || normalInd > normals.size()) {
                        std::cerr << "Error parsing directive in " << objFilePath << " on line " << faceData.defLineNum << ": " << faceData.defLine << std::endl;
                        std::cerr << "Invalid normal index " << normalInd << " (" << normals.size() << " normals defined)" << std::endl;
                        return std::vector<GLfloat>();
                    }
                    vertexData.push_back(normals[normalInd - 1].x);
                    vertexData.push_back(normals[normalInd - 1].y);
                    vertexData.push_back(normals[normalInd - 1].z);
                } else {
                    vertexData.push_back(calcNormal.x);
                    vertexData.push_back(calcNormal.y);
                    vertexData.push_back(calcNormal.z);
                }
            }
        }

        std::cout << "Finished parsing " << objFilePath << "!" << std::endl;
        return vertexData;
    }
private:
    static std::vector<std::string> tokenize(std::string line, std::string delims) {
        std::vector<std::string> tokens;
        size_t start = 0, end;
        while ((end = line.find_first_of(delims, start)) != std::string::npos) {
            tokens.push_back(line.substr(start, end - start));
            start = end + 1;
        }
        if (start < line.size()) {
            tokens.push_back(line.substr(start));
        }
        return tokens;
    }

    // Parses the given string as a float, returning whether it was successful
    static bool parseAsFloat(const std::string& str, GLfloat* result) {
        try {
            size_t idx;
            *result = std::stof(str, &idx);
            if(idx != str.size()) {
                return false;
            }
            return true;
        } catch (const std::invalid_argument&) {}
        catch (const std::out_of_range&) {}
        return false;
    }

    // Parses the given string as an int, returning whether it was successful
    static bool parseAsInt(const std::string& str, int* result) {
        try {
            size_t idx;
            *result = std::stoi(str, &idx);
            if(idx != str.size()) {
                return false;
            }
            return true;
        } catch (const std::invalid_argument&) {}
        catch (const std::out_of_range&) {}
        return false;
    }
};

#endif // OBJLOADER_H
