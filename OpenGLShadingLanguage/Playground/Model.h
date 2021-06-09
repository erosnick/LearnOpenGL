#pragma once

#include <memory>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Texture.h"
#include "Material.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 tangent;
    glm::vec3 binormal;
    glm::vec3 normal;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const {
        return (other.position == position) &&
            (other.normal == normal) &&
            (other.texCoord == texCoord);
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position)
                ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1)
                ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

//namespace std {
//    template<> struct hash<Vertex> {
//        size_t operator()(Vertex const& vertex) const {
//            return (hash<glm::vec3>()(vertex.position))
//                 ^ (hash<glm::vec3>()(vertex.normal) << 1)
//                 ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
//        }
//    };
//}

class Model {
public:
    Model() {
        position = glm::vec3(0.0f);
        transform = glm::mat4(1.0f);
    }

    ~Model() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteBuffers(1, &vao);
    }

    void addVertex(const Vertex& vertex) {
        vertices.push_back(vertex);
    }

    void addIndex(uint32_t index) {
        indices.push_back(index);
    }

    size_t getVertexBufferByteSize() const {
        return sizeof(Vertex) * vertices.size();
    }

    size_t getIndexBufferByteSize() const {
        return sizeof(uint32_t) * indices.size();
    }

    uint32_t getElementCount() const {
        return static_cast<uint32_t>(indices.size());
    }

    uint32_t getTriangleCount() const {
        return static_cast<uint32_t>(vertices.size() / 3);
    }

    void scale(const glm::vec3& factor) {
        transform = glm::scale(transform, factor);
    }

    void translate(const glm::vec3& offset) {
        transform = glm::translate(transform, offset);
    }

    void rotate(float angle, const glm::vec3& axis) {
        transform = glm::rotate(transform, glm::radians(angle), axis);
    }

    void setPosition(const glm::vec3& inPosition) {
        position = inPosition;
        transform[3][0] = inPosition.x;
        transform[3][1] = inPosition.y;
        transform[3][2] = inPosition.z;
    }

    glm::vec3 getPosition() const {
        return position;
    }

    glm::mat4 getTransform() const {
        return transform;
    }

    std::vector<Vertex> getVertices() const {
        return vertices;
    }

    const Vertex* getVerticesData() const {
        return vertices.data();
    }

    std::vector<uint32_t> getIndices() const {
        return indices;
    }

    const uint32_t* getIndicesData() const {
        return indices.data();
    }

    void computeTangentSpace();

    void prepareDraw();

    void use() {
        glBindVertexArray(vao);
    }

    void addTexture(const std::shared_ptr<Texture>& texture) {
        textures.push_back(texture);
    }

    int32_t getTextureIndex(uint32_t index) const {
        if (index >= textures.size()) {
            return 0;
        }

        auto texture = textures[index];

        if (texture) {
            return texture->getTextureIndex();
        }

        return 0;
    }

    void setMaterial(const std::shared_ptr<Material>& inMaterial) {
        material = inMaterial;
    }

    const std::shared_ptr<Material>& getMaterial() const {
        return material;
    }

    void setName(const std::string& inName) {
        name = inName;
    }

    const std::string& getName() const {
        return name;
    }

private:

    std::string name;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    glm::vec3 position;
    glm::mat4 transform;

    uint32_t vbo = -1;
    uint32_t ibo = -1;
    uint32_t vao = -1;

    std::shared_ptr<Material> material;
    std::vector<std::shared_ptr<Texture>> textures;
};
