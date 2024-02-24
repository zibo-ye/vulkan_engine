#pragma once

#include "pch.hpp"

class VulkanCore;

using ExtentVariant = std::variant<uint32_t, VkExtent2D, VkExtent3D>;

class Image {
public:
    bool Init(VulkanCore* pVulkanCore, ExtentVariant extent, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    void Destroy();

    void InitImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
    void TransitionLayout(std::optional<VkCommandBuffer> commandBuffer, VkImageLayout newLayout, uint32_t mipLevels);
    ExtentVariant GetImageExtent();

public:
    bool m_isValid = false;

    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;
    // VkSampler sampler;
    VkImageCreateInfo m_imageInfo;
    VkImageLayout m_currentLayout;

    VulkanCore* m_pVulkanCore;
};
