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
