#pragma once
#include "MeshData.hpp"
#include "Scene.hpp"
#include "SceneEnum.hpp"
#include "SceneObj.hpp"
#include "pch.hpp"
#include <limits>

struct MeshIndices {
    MeshIndices() = default;
    MeshIndices(const Utility::json::JsonValue& jsonObj);

    std::string src;
    size_t offset = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
};

struct MeshAttributes {
    MeshAttributes() = default;
    MeshAttributes(const Utility::json::JsonValue& jsonObj, const std::string& scenePath);

    std::string src;
    size_t offset = 0;
    size_t stride = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
};

enum class MeshAttributeType {
    POSITION,
    NORMAL,
    COLOR,
    UV,
    TANGENT,
    BITANGENT,
    JOINTS,
    WEIGHTS,
    CUSTOM
};

struct NewVertex {
    vkm::vec3 position;
    vkm::vec3 normal;
    vkm::u8vec4 color;

    bool operator==(const NewVertex& other) const
    {
        return position == other.position && normal == other.normal && color == other.color;
    }

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription {
            .binding = 0,
            .stride = sizeof(NewVertex),
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
                .offset = offsetof(NewVertex, position),
            },
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(NewVertex, normal),
            },
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .offset = offsetof(NewVertex, color),
            },
        };

        return attributeDescriptions;
    }

    static int getAttributeOffset(MeshAttributeType type)
    {
        switch (type) {
        case MeshAttributeType::POSITION:
            return offsetof(NewVertex, position);
        case MeshAttributeType::NORMAL:
            return offsetof(NewVertex, normal);
        case MeshAttributeType::COLOR:
            return offsetof(NewVertex, color);
        default:
            return -1;
        }
    }

    static int getAttributeOffset(const std::string& type)
    {
        if (type == "POSITION")
            return offsetof(NewVertex, position);
        else if (type == "NORMAL")
            return offsetof(NewVertex, normal);
        else if (type == "COLOR")
            return offsetof(NewVertex, color);
        else
            return -1;
    }
};

namespace std {
template <>
struct hash<NewVertex> {
    size_t operator()(const NewVertex& vertex) const
    {
        size_t h1 = hash<vkm::vec3>()(vertex.position);
        size_t h2 = hash<vkm::vec3>()(vertex.normal);
        size_t h3 = hash<vkm::u8vec4>()(vertex.color);

        // Combine the hash values
        return ((h1 ^ (h2 << 1)) >> 1) ^ h3;
    }
};
}

class Mesh : public SceneObj {
public:
    Mesh(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);

    std::string name;
    VkPrimitiveTopology topology;
    size_t count;
    std::optional<MeshIndices> indiceDescription;
    std::unordered_map<std::string, MeshAttributes> attributeDescriptions;

    std::shared_ptr<MeshData<NewVertex, uint32_t>> meshData;
    void LoadMeshData();

public:
    vkm::vec3 min = vkm::vec3(std::numeric_limits<float>::max());
    vkm::vec3 max = -min;

private:
    void UpdateBounds(const NewVertex& vertex);
};

inline static VkPipelineVertexInputStateCreateInfo getVertexInputInfo()
{
    // Currently, we assume that all the attributes have the same stride and offset.
    auto bindingDescription = NewVertex::getBindingDescription();
    auto attributeDescriptions = NewVertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    return vertexInputInfo;
}

struct MeshInstance {
    std::shared_ptr<Mesh> pMesh;
    vkm::mat4 matWorld;
};
