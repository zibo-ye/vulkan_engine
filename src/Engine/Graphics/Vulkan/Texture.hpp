#pragma once

#include "pch.hpp"

class VulkanCore;

class Texture {

public:
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
    VkSampler sampler;
    uint32_t mipLevels;
    uint32_t texWidth;
    uint32_t texHeight;

    std::shared_ptr<VulkanCore> vulkanCore;
};