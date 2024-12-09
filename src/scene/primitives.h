#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <functional>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <GL/glew.h>
#include <QImage>
#include "utils/scenedata.h"
#include "utils/debug.h"

// ----------------------------------------------------------------------------------------------------------
// Types used for tessellation and texturing

// Converts an object-space position to an object-space normal
typedef std::function<glm::vec3(glm::vec3)> getNormalFunc;

// Converts an object-space position to UV coordinates
typedef std::function<glm::vec2(glm::vec3)> getUVCartFunc;

// Caches a map from filenames to textures, initialized in scene.cpp
extern std::unordered_map<std::string, QImage> fileToTexture;

// ----------------------------------------------------------------------------------------------------------
// Primitive class

// Represents a generic primitive (mesh)
class Primitive {
public:
    Primitive(const glm::mat4&ctm, const SceneMaterial& material) {
        this->objectToWorld = ctm;
        this->worldToObject = glm::inverse(ctm);
        this->objNormalToWorld = glm::inverse(glm::transpose(glm::mat3(this->objectToWorld)));
        this->material = material;
        this->vertexData = std::vector<float>();
    }

    // Constructs a primitive with manually specified vertex data
    Primitive(const glm::mat4&ctm, const SceneMaterial& material, std::vector<GLfloat>& vertexData)
            : Primitive(ctm, material) {
        this->vertexData = vertexData;
    }

    // Frees any underlying memory for used OpenGL objects
    virtual ~Primitive() {
        glErrorCheck(glDeleteBuffers(1, &this->vbo));
        glErrorCheck(glDeleteVertexArrays(1, &this->vao));
        glErrorCheck(glDeleteTextures(1, &this->texture));
    }

    // Initializes any OpenGL-related objects for the primitive
    const inline void initialize() {
        // Generate vertex data
        calcVertexData();

        // Generate VBO and VAO objects and texture
        glErrorCheck(glGenBuffers(1, &this->vbo));
        glErrorCheck(glGenVertexArrays(1, &this->vao));
        glErrorCheck(glGenTextures(1, &this->texture));

        // Bind VBO
        glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
        glErrorCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->vertexData.size(), this->vertexData.data(), GL_STATIC_DRAW));

        // Bind VAO
        glErrorCheck(glBindVertexArray(this->vao));

        // Add attributes to VAO
        glErrorCheck(glEnableVertexAttribArray(0));
        glErrorCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat))));

        glErrorCheck(glEnableVertexAttribArray(1));
        glErrorCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat))));

        glErrorCheck(glEnableVertexAttribArray(2));
        glErrorCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat))));

        // Unbind VBO and VAO
        glErrorCheck(glBindVertexArray(0));
        glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));

        // Initialize texture, if textured
        SceneFileMap& textureMap = this->material.textureMap;
        if(textureMap.isUsed) {
            glErrorCheck(glActiveTexture(GL_TEXTURE0));
            glErrorCheck(glBindTexture(GL_TEXTURE_2D, this->texture));
            QImage& textureImage = fileToTexture[textureMap.filename];
            glErrorCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage.width(), textureImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureImage.bits()));
            glErrorCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            glErrorCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            glErrorCheck(glBindTexture(GL_TEXTURE_2D, 0));
        }
    }

    // Calculates the vertex data of the given primitive, according to its shape parameters
    const virtual void calcVertexData() {}

    // Draws the given primitive
    const inline void draw(GLuint shader, bool shadowMap) const {
        // Bind any uniform variables associated with this primitive
        glErrorCheck(glUniformMatrix4fv(glGetUniformLocation(shader, "modelMat"), 1, false, &this->objectToWorld[0][0]));
        if(!shadowMap) {
            glErrorCheck(glUniformMatrix3fv(glGetUniformLocation(shader, "normalMat"), 1, false, &this->objNormalToWorld[0][0]));

            // Pass material properties as a uniform
            glErrorCheck(glUniform4fv(glGetUniformLocation(shader, "materialAmbient"), 1, &this->material.cAmbient[0]));
            glErrorCheck(glUniform4fv(glGetUniformLocation(shader, "materialDiffuse"), 1, &this->material.cDiffuse[0]));
            glErrorCheck(glUniform4fv(glGetUniformLocation(shader, "materialSpecular"), 1, &this->material.cSpecular[0]));
            glErrorCheck(glUniform1f(glGetUniformLocation(shader, "materialShininess"), this->material.shininess));

            // Pass texture properties as a uniform
            glErrorCheck(glUniform1i(glGetUniformLocation(shader, "textured"), this->material.textureMap.isUsed));
            glErrorCheck(glActiveTexture(GL_TEXTURE0));
            glErrorCheck(glBindTexture(GL_TEXTURE_2D, this->texture));
            glErrorCheck(glUniform1f(glGetUniformLocation(shader, "materialBlend"), this->material.blend));
        }

        // Bind VAO and draw associated vertices
        glErrorCheck(glBindVertexArray(this->vao));

        glErrorCheck(glDrawArrays(GL_TRIANGLES, 0, this->vertexData.size() / 8));

        glErrorCheck(glBindVertexArray(0));
        glErrorCheck(glBindTexture(GL_TEXTURE_2D, 0));
    }

protected:
    glm::mat4 objectToWorld;
    glm::mat4 worldToObject;
    glm::mat3 objNormalToWorld;
    SceneMaterial material;

    GLuint vbo, vao, texture;
    std::vector<GLfloat> vertexData;
};


// A cube primitive; centered at the origin, with sides of length 1
class Cube : public Primitive {
public:
    Cube(const glm::mat4& ctm, const SceneMaterial& material, int param, bool inverted) : Primitive(ctm, material) {
        this->param = param;
        this->inverted = inverted;
    }

    const void calcVertexData() override;
protected:
    void makeTile(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
                  std::vector<glm::vec2>& uvs);
    const void makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                        glm::vec3 bottomLeft, glm::vec3 bottomRight,
                        getNormalFunc getNormal, getUVCartFunc getUV);

    const float boundCoord = 0.5f;
    int param;
    bool inverted;
};

#endif // PRIMITIVES_H
