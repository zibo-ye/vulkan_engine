#pragma once
#include "pch.hpp"

class VulkanCore;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {
            VkVertexInputAttributeDescription {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, pos),
            },
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord),
            },
        };

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const
    {
        return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};
}

class Model {
public:
    bool loadModelFromFile(std::string path);

public:
    const std::vector<Vertex>& getVertices() { return vertices; }
    const std::vector<uint32_t>& getIndices() { return indices; }

private:
    bool isValid = false;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

public:
    bool uploadModelToGPU(const VulkanCore* vulkanCore);
    bool releaseModelFromGPU();

    VkBuffer GetVertexBuffer()
    {
        return vertexBuffer;
    }
    VkBuffer GetIndexBuffer()
    {
        return indexBuffer;
    }

private:
    void createVertexBuffer(const VulkanCore* vulkanCore);
    void createIndexBuffer(const VulkanCore* vulkanCore);
private:
    bool isOnGPU = false;
    const VulkanCore* m_pVulkanCore = nullptr;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
};
