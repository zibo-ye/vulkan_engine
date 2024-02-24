#pragma once

#include "pch.hpp"

class VulkanCore;

class Image {

public:
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;
    VkFormat imageFormat;
    VkSampler sampler;
    uint32_t mipLevels;
    VkExtent3D imageExtent;

    std::shared_ptr<VulkanCore> vulkanCore;
};
