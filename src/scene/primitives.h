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

// A struct containing all parameters that could affect tessellation
struct TessellationParams {
    int param1;                 // shape parameter 1
    int param2;                 // shape parameter 2
    bool limitByNum;            // whether to limit tessellation by number of primitives in the scene
    std::size_t numPrimitives;  // number of primitives in the scene
    bool limitByDist;           // whether to limit tesellation by distance to the camera
    glm::vec4 cameraPos;        // the position of the camera in world space
    float nearPlane;            // distance to the near plane
    float farPlane;             // distance to the far plane
};
// Converts an object-space position to an object-space normal
typedef std::function<glm::vec3(glm::vec3)> getNormalFunc;

// Converts an object-space position to UV coordinates
typedef std::function<glm::vec2(float, float, float)> getUVFunc;
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

        // Generate VBO and VAO objects
        glErrorCheck(glGenBuffers(1, &this->vbo));
        glErrorCheck(glGenVertexArrays(1, &this->vao));
        glErrorCheck(glGenTextures(1, &this->texture));
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

    // Updates the vertices of the given primitive based on the parameters
    const virtual void updateParams(TessellationParams& tslParams) {}

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



// ----------------------------------------------------------------------------------------------------------
// TessellatedPrimitive class

// Represents a tessellated primitive
class TessellatedPrimitive : public Primitive {
public:
    TessellatedPrimitive(const glm::mat4& ctm, const SceneMaterial& material,
                         TessellationParams& tslParams, int minParam1, int minParam2)
            : Primitive(ctm, material) {
        this->tslParams = tslParams;
        this->tslParams.param1 = std::max(tslParams.param1, minParam1);
        this->tslParams.param2 = std::max(tslParams.param2, minParam2);
        this->minParam1 = minParam1;
        this->minParam2 = minParam2;
        this->adjParam1 = adjustParam(this->tslParams.param1, this->minParam1, this->tslParams);
        this->adjParam2 = adjustParam(this->tslParams.param2, this->minParam2, this->tslParams);
    }

    // Updates the vertices of the given primitive based on the parameters
    const void updateParams(TessellationParams& tslParams) {
        int actParam1 = std::max(tslParams.param1, this->minParam1);
        int actParam2 = std::max(tslParams.param2, this->minParam2);
        bool changeVerts = this->tslParams.param1 != actParam1 || this->tslParams.param2 != actParam2 ||
                           this->tslParams.limitByNum != tslParams.limitByNum ||
                           (tslParams.limitByNum && this->tslParams.numPrimitives != tslParams.numPrimitives) ||
                           this->tslParams.limitByDist != tslParams.limitByDist ||
                           (tslParams.limitByDist &&
                            (this->tslParams.cameraPos != tslParams.cameraPos ||
                             this->tslParams.nearPlane != tslParams.nearPlane ||
                             this->tslParams.farPlane != tslParams.farPlane));
        this->tslParams = tslParams;
        this->tslParams.param1 = actParam1;
        this->tslParams.param2 = actParam2;
        if(changeVerts) {
            this->adjParam1 = adjustParam(this->tslParams.param1, this->minParam1, this->tslParams);
            this->adjParam2 = adjustParam(this->tslParams.param2, this->minParam2, this->tslParams);
            // Generate new vertex data
            calcVertexData();

            // Associate new data with VBO
            glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, this->vbo));
            glErrorCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * this->vertexData.size(), this->vertexData.data(), GL_STATIC_DRAW));
            glErrorCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }
    }

protected:
    TessellationParams tslParams;
    int adjParam1, adjParam2;
    int minParam1, minParam2;

    // Makes and inserts a quadrilateral tile given by the given vertices
    // (in order: top-left, top-right, bottom-left, bottom-right)
    // in counterclockwise order into the primitive's vertex data.
    // Also inserts the corresponding normals and UV-coordinates for each vertex.
    void makeTile(std::vector<glm::vec3>& vertices, std::vector<glm::vec3>& normals,
                  std::vector<glm::vec2>& uvs);

    // Adjusts the shape parameter depending on adaptive level of detail
    int adjustParam(int param, int minParam, TessellationParams& tslParams);
};


// ----------------------------------------------------------------------------------------------------------
// Inheritors of the TessellatedPrimitive class

// A sphere centered at the origin, with a diameter of 1
class Sphere : public TessellatedPrimitive {
public:
    Sphere(const glm::mat4& ctm, const SceneMaterial& material, TessellationParams& tslParams)
        : TessellatedPrimitive(ctm, material, tslParams, 2, 3) {}

    const void calcVertexData() override;
private:
    const float radius = 0.5f;
    const getNormalFunc getSphereNormal = [](glm::vec3 pos) -> glm::vec3 {
        return pos;
    };
    const getUVFunc getSphereUV = [](float r, float theta, float phi) -> glm::vec2 {
        return glm::vec2(1 - theta / (2 * M_PI), 1 - phi / M_PI);
    };
};


// A cube centered at the origin, with sides of length 1
class Cube : public TessellatedPrimitive {
public:
    Cube(const glm::mat4& ctm, const SceneMaterial& material, TessellationParams& tslParams)
        : TessellatedPrimitive(ctm, material, tslParams, 1, 1) {}

    const void makeFace(glm::vec3 topLeft, glm::vec3 topRight,
                        glm::vec3 bottomLeft, glm::vec3 bottomRight,
                        getNormalFunc getNormal, getUVCartFunc getUV);
    const void calcVertexData() override;
private:
    const float boundCoord = 0.5f;
};


// A cylinder centered at the origin with a height and diameter of 1
class Cylinder : public TessellatedPrimitive {
public:
    Cylinder(const glm::mat4& ctm, const SceneMaterial& material, TessellationParams& tslParams)
        : TessellatedPrimitive(ctm, material, tslParams, 1, 3) {}

    const void calcVertexData() override;
private:
    const float height = 1;
    const float radius = 0.5f;
    const getNormalFunc getSideNormal = [](glm::vec3 pos) -> glm::vec3 {
        return glm::vec3(pos.x, 0, pos.z);
    };
    const getUVFunc getSideUV = [](float r, float theta, float z) -> glm::vec2 {
        return glm::vec2(1 - theta / (2 * M_PI), z + 0.5);
    };
    const getNormalFunc getBaseNormal = [](glm::vec3 pos) -> glm::vec3 {
        return glm::vec3(0, -1, 0);
    };
    const getUVCartFunc getBaseUV = [](glm::vec3 pos) -> glm::vec2 {
        return glm::vec2(pos.x + 0.5, pos.z + 0.5);
    };
    const getNormalFunc getTopNormal = [](glm::vec3 pos) -> glm::vec3 {
        return glm::vec3(0, 1, 0);
    };
    const getUVCartFunc getTopUV = [](glm::vec3 pos) -> glm::vec2 {
        return glm::vec2(pos.x + 0.5, 0.5 - pos.z);
    };
};


// A cone centered at the origin with a height of 1 and whose base has a diameter of 1
class Cone : public TessellatedPrimitive {
public:
    Cone(const glm::mat4& ctm, const SceneMaterial& material, TessellationParams& tslParams)
        : TessellatedPrimitive(ctm, material, tslParams, 1, 3) {}

    const void calcVertexData() override;
private:
    const float height = 1;
    const float baseRadius = 0.5f;
    const getNormalFunc getSideNormal = [](glm::vec3 pos) -> glm::vec3 {
        return glm::vec3(pos.x, (1 - 2 * pos.y) / 8, pos.z);
    };
    const getUVFunc getSideUV = [](float r, float theta, float z) -> glm::vec2 {
        return glm::vec2(1 - theta / (2 * M_PI), z + 0.5);
    };
    const getNormalFunc getBaseNormal = [](glm::vec3 pos) -> glm::vec3 {
        return glm::vec3(0, -1, 0);
    };
    const getUVCartFunc getBaseUV = [](glm::vec3 pos) -> glm::vec2 {
        return glm::vec2(pos.x + 0.5, pos.z + 0.5);
    };
};

#endif // PRIMITIVES_H
