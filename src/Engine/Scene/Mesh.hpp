#pragma once
#include "SceneEnum.hpp"
#include "SceneObj.hpp"
#include "Scene.hpp"
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

//struct NewVertex {
//    glm::vec3 position;
//    glm::vec3 normal;
//    glm::u8vec4 color;
//
//    static VkVertexInputBindingDescription getBindingDescription()
//    {
//        VkVertexInputBindingDescription bindingDescription {
//            .binding = 0,
//            .stride = sizeof(NewVertex),
//            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
//        };
//
//        return bindingDescription;
//    }
//
//    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
//    {
//        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {
//            VkVertexInputAttributeDescription {
//                .location = 0,
//                .binding = 0,
//                .format = VK_FORMAT_R32G32B32_SFLOAT,
//                .offset = offsetof(NewVertex, position),
//            },
//            {
//                .location = 1,
//                .binding = 0,
//                .format = VK_FORMAT_R32G32B32_SFLOAT,
//                .offset = offsetof(NewVertex, normal),
//            },
//            {
//                .location = 2,
//                .binding = 0,
//                .format = VK_FORMAT_R8G8B8A8_UNORM,
//                .offset = offsetof(NewVertex, color),
//            },
//        };
//
//        return attributeDescriptions;
//    }
//};


class Mesh : public SceneObj {
public:
    Mesh(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);

    std::string name;
    VkPrimitiveTopology topology;
    size_t count;
    std::optional<MeshIndices> indices;

    std::unordered_map<std::string, MeshAttributes> attributes;

    //VkPipelineVertexInputStateCreateInfo getVertexInputInfo() const;
};
