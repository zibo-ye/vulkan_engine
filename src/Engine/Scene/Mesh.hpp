#pragma once
#include "MeshData.hpp"
#include "Scene.hpp"
#include "SceneEnum.hpp"
#include "SceneObj.hpp"
#include "pch.hpp"

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
    TEXCOORD,
    TANGENT,
    BITANGENT,
    JOINTS,
    WEIGHTS,
    CUSTOM
};

struct NewVertex {
    vkm::vec3 position; // POSITION: position stream
    vkm::vec3 normal; // NORMAL: vertex normal
    vkm::vec4 tangent; // TANGENT: tangent (xyz) + bitangent sign (w)
    vkm::vec2 texCoord; // TEXCOORD: texture coordinates
    vkm::u8vec4 color; // COLOR: vertex color

    bool operator==(const NewVertex& other) const
    {
        return position == other.position && normal == other.normal && color == other.color && texCoord == other.texCoord && tangent == other.tangent;
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

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions {
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
                .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                .offset = offsetof(NewVertex, tangent),
            },
            {
                .location = 3,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(NewVertex, texCoord),
            },
            {
                .location = 4,
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
        case MeshAttributeType::TEXCOORD:
            return offsetof(NewVertex, texCoord);
        case MeshAttributeType::TANGENT:
            return offsetof(NewVertex, tangent);
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
        else if (type == "TEXCOORD")
            return offsetof(NewVertex, texCoord);
        else if (type == "TANGENT")
            return offsetof(NewVertex, tangent);
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
        size_t h4 = hash<vkm::vec2>()(vertex.texCoord);
        size_t h5 = hash<vkm::vec4>()(vertex.tangent);

        // Combine the hash values
        return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
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
    std::optional<int> materialIdx;

    std::shared_ptr<MeshData<NewVertex, uint32_t>> meshData;
    void LoadMeshData();

public:
    vkm::vec3 min = vkm::vec3(std::numeric_limits<float>::max());
    vkm::vec3 max = -min;

private:
    void UpdateBounds(const NewVertex& vertex);

public:
    EMaterialType GetMaterialType() const;
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
