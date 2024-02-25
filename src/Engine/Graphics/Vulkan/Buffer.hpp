#pragma once

#include "pch.hpp"

class Image;
class VulkanCore;

using ExtentVariant = std::variant<uint32_t, VkExtent2D, VkExtent3D>;

class Buffer {
public:
    bool Init(const VulkanCore* pVulkanCore, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool alwaysMap = false);
    void Destroy();
    void* Map();
    void Unmap();
    void UploadData(const void* data, VkDeviceSize size);
    void CopyToBuffer(Buffer& dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0);
    void CopyToImage(Image image, uint32_t width, uint32_t height);

public:
    bool m_isValid = false;

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    VkBufferView bufferView;
    VkBufferCreateInfo m_bufferInfo;

    bool m_mappable = false;
    std::optional<void*> m_pMappedData = std::nullopt;

    const VulkanCore* m_pVulkanCore;
};
