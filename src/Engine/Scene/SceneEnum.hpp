#pragma once

#include "pch.hpp"

enum class ESceneObjType {
    MESH,
    NODE,
    CAMERA,
    DRIVER
};
ESceneObjType GetSceneObjType(const std::string& typeStr);

enum EDriverChannelType {
    TRANSLATION,
    ROTATION,
    SCALE,
};
EDriverChannelType GetDriverChannelType(const std::string& typeStr);

enum EDriverInterpolationType {
    STEP,
    LINEAR,
    SLERP
};
EDriverInterpolationType GetDriverInterpolationType(const std::string& typeStr);

VkPrimitiveTopology GetVkPrimitiveTopology(const std::string& typeStr);

VkFormat GetVkFormat(const std::string& typeStr);
int GetVkFormatByteSize(VkFormat format);