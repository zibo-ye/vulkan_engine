
#include "SceneEnum.hpp"
#include "pch.hpp"

ESceneObjType GetSceneObjType(const std::string& typeStr)
{
    if (typeStr == "MESH") {
        return ESceneObjType::MESH;
    } else if (typeStr == "NODE") {
        return ESceneObjType::NODE;
    } else if (typeStr == "CAMERA") {
        return ESceneObjType::CAMERA;
    } else if (typeStr == "DRIVER") {
        return ESceneObjType::DRIVER;
    } else {
        throw std::runtime_error("Unknown scene object type: " + typeStr);
    }
}

EDriverChannelType GetDriverChannelType(const std::string& typeStr)
{
    if (typeStr == "translation") {
        return EDriverChannelType::TRANSLATION;
    } else if (typeStr == "rotation") {
        return EDriverChannelType::ROTATION;
    } else if (typeStr == "scale") {
        return EDriverChannelType::SCALE;
    } else {
        throw std::runtime_error("Unknown driver channel type: " + typeStr);
    }
}

EDriverInterpolationType GetDriverInterpolationType(const std::string& typeStr)
{
    if (typeStr == "STEP") {
        return EDriverInterpolationType::STEP;
    } else if (typeStr == "LINEAR") {
        return EDriverInterpolationType::LINEAR;
    } else if (typeStr == "SLERP") {
        return EDriverInterpolationType::SLERP;
    } else {
        throw std::runtime_error("Unknown driver interpolation type: " + typeStr);
    }
}

VkPrimitiveTopology GetVkPrimitiveTopology(const std::string& typeStr)
{
    if (typeStr == "POINT_LIST") {
        return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    } else if (typeStr == "LINE_LIST") {
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    } else if (typeStr == "LINE_STRIP") {
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    } else if (typeStr == "TRIANGLE_LIST") {
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    } else if (typeStr == "TRIANGLE_STRIP") {
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    } else if (typeStr == "TRIANGLE_FAN") {
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    } else if (typeStr == "LINE_LIST_WITH_ADJACENCY") {
        return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    } else if (typeStr == "LINE_STRIP_WITH_ADJACENCY") {
        return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    } else if (typeStr == "TRIANGLE_LIST_WITH_ADJACENCY") {
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    } else if (typeStr == "TRIANGLE_STRIP_WITH_ADJACENCY") {
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    } else if (typeStr == "PATCH_LIST") {
        return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    } else {
        throw std::runtime_error("Unknown primitive topology: " + typeStr);
    }
}

VkFormat GetVkFormat(const std::string& typeStr)
{
    if (typeStr == "R32_SFLOAT") {
        return VK_FORMAT_R32_SFLOAT;
    } else if (typeStr == "R32G32_SFLOAT") {
        return VK_FORMAT_R32G32_SFLOAT;
    } else if (typeStr == "R32G32B32_SFLOAT") {
        return VK_FORMAT_R32G32B32_SFLOAT;
    } else if (typeStr == "R32G32B32A32_SFLOAT") {
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    } else if (typeStr == "R8_UNORM") {
        return VK_FORMAT_R8_UNORM;
    } else if (typeStr == "R8G8_UNORM") {
        return VK_FORMAT_R8G8_UNORM;
    } else if (typeStr == "R8G8B8_UNORM") {
        return VK_FORMAT_R8G8B8_UNORM;
    } else if (typeStr == "R8G8B8A8_UNORM") {
        return VK_FORMAT_R8G8B8A8_UNORM;
    } else {
        throw std::runtime_error("Unknown format: " + typeStr);
        return VK_FORMAT_UNDEFINED;
    }
}
