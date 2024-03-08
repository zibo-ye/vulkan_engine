#pragma once

#include "pch.hpp"

class Buffer;
class VulkanCore;

using ExtentVariant = std::variant<uint32_t, VkExtent2D, VkExtent3D>;

class Image {
public:
    bool Init(VulkanCore* pVulkanCore, ExtentVariant extent, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool isCube = false);
    void Destroy();

    void InitImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D);
    void createTextureSampler();
    void TransitionLayout(std::optional<VkCommandBuffer> commandBuffer, VkImageLayout newLayout);
    ExtentVariant GetImageExtent();

public:
    void CopyToBuffer(Buffer& buffer);

    std::unique_ptr<uint8_t[]> copyToMemory();
    void UploadData(const void* data, VkDeviceSize size); // #TODO
    void GenerateMipmaps(std::optional<VkCommandBuffer> commandBuffer, uint32_t mipLevels);

public:
    VkDescriptorImageInfo GetDescriptorImageInfo()
    {
        VkDescriptorImageInfo imageInfo {
            .sampler = sampler,
            .imageView = imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // TODO: Can this be m_currentLayout?
        };

        return imageInfo;
    }

public:
    bool m_isValid = false;

    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;
    VkSampler sampler;
    VkImageCreateInfo m_imageInfo;
    VkImageViewCreateInfo m_imageViewInfo;
    VkImageLayout m_currentLayout;

    VulkanCore* m_pVulkanCore;
    bool m_isCube = false;
};
