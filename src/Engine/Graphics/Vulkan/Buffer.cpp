#include "Buffer.hpp"
#include "VulkanCore.hpp"

bool Buffer::Init(const VulkanCore* pVulkanCore, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, bool alwaysMap /*= false*/)
{
    if (!pVulkanCore)
        return false;
    m_pVulkanCore = pVulkanCore;

    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VK(vkCreateBuffer(m_pVulkanCore->GetDevice(), &bufferInfo, nullptr, &buffer));
    m_bufferInfo = std::move(bufferInfo);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_pVulkanCore->GetDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = m_pVulkanCore->findMemoryType(memRequirements.memoryTypeBits, properties),
    };

    VK(vkAllocateMemory(m_pVulkanCore->GetDevice(), &allocInfo, nullptr, &bufferMemory))

    vkBindBufferMemory(m_pVulkanCore->GetDevice(), buffer, bufferMemory, 0);
    m_isValid = true;
    m_mappable = (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    if (m_mappable && alwaysMap)
        m_pMappedData = Map();
    return true;
}

void Buffer::Destroy()
{
    if (m_isValid) {
        if (m_pMappedData)
            Unmap();
        vkDestroyBuffer(m_pVulkanCore->GetDevice(), buffer, nullptr);
        vkFreeMemory(m_pVulkanCore->GetDevice(), bufferMemory, nullptr);
        m_isValid = false;
    }
}

void* Buffer::Map()
{
    if (!m_mappable)
        return nullptr;
    void* data;
    VK(vkMapMemory(m_pVulkanCore->GetDevice(), bufferMemory, 0, m_bufferInfo.size, 0, &data));
    return data;
}

void Buffer::Unmap()
{
    if (m_mappable)
        vkUnmapMemory(m_pVulkanCore->GetDevice(), bufferMemory);
}

void Buffer::CopyToBuffer(Buffer& dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset /*= 0*/, VkDeviceSize dstOffset /*= 0*/)
{
    VkCommandBuffer commandBuffer = m_pVulkanCore->beginSingleTimeCommands();

    VkBufferCopy copyRegion {
        .srcOffset = srcOffset,
        .dstOffset = dstOffset,
        .size = size
    };

    vkCmdCopyBuffer(commandBuffer, this->buffer, dstBuffer.buffer, 1, &copyRegion);

    m_pVulkanCore->endSingleTimeCommands(commandBuffer);
}

void Buffer::UploadData(const void* data, VkDeviceSize size)
{
    // Create a staging buffer
    Buffer stagingBuffer;
    stagingBuffer.Init(m_pVulkanCore, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Map memory and copy data to the staging buffer
    void* mappedData = stagingBuffer.Map();
    memcpy(mappedData, data, static_cast<size_t>(size));
    stagingBuffer.Unmap();

    stagingBuffer.CopyToBuffer(*this, size);

    // Clean up the staging buffer
    stagingBuffer.Destroy();
}

void Buffer::CopyToImage(Image& image)
{
    VkCommandBuffer commandBuffer = m_pVulkanCore->beginSingleTimeCommands();

    VkBufferImageCopy region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,

        .imageSubresource {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0, // TODO: support mipmaps
            .baseArrayLayer = 0,
            .layerCount = 1,
        },

        .imageOffset = { 0, 0, 0 },
        .imageExtent = image.m_imageInfo.extent,
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        this->buffer,
        image.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    m_pVulkanCore->endSingleTimeCommands(commandBuffer);
}
