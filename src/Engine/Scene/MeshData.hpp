#pragma once
#include "Graphics/Vulkan/VulkanCore.hpp"
#include "pch.hpp"

template <typename VertexType, typename IndexType = uint32_t>
class MeshData {
public:
    MeshData() = default;
    ~MeshData()
    {
        if (isOnGPU)
            releaseModelFromGPU();
    }
    std::vector<VertexType> vertices;
    std::optional<std::vector<IndexType>> indices;

public:
    bool uploadModelToGPU(const VulkanCore* vulkanCore);
    bool releaseModelFromGPU();

    VkBuffer vertexBuffer;
    std::optional<VkBuffer> indexBuffer;
    // #TODO: buffer's lifecycle is limited by vulkanCore.
private:
    void createVertexBuffer(const VulkanCore* vulkanCore);
    void createIndexBuffer(const VulkanCore* vulkanCore);

private:
    bool isOnGPU = false;
    const VulkanCore* m_pVulkanCore = nullptr;
    VkDeviceMemory vertexBufferMemory;
    std::optional<VkDeviceMemory> indexBufferMemory;
};

template <typename VertexType, typename IndexType /*= uint32_t*/>
void MeshData<VertexType, IndexType>::createIndexBuffer(const VulkanCore* vulkanCore)
{
    if (!indices.has_value())
        return;
    VkDeviceSize bufferSize = sizeof(indices.value()[0]) * indices.value().size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkanCore->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vulkanCore->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.value().data(), (size_t)bufferSize);
    vkUnmapMemory(vulkanCore->GetDevice(), stagingBufferMemory);

    indexBuffer = VkBuffer();
    indexBufferMemory = VkDeviceMemory();

    vulkanCore->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer.value(), indexBufferMemory.value());

    vulkanCore->copyBuffer(stagingBuffer, indexBuffer.value(), bufferSize);

    vkDestroyBuffer(vulkanCore->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanCore->GetDevice(), stagingBufferMemory, nullptr);
}

template <typename VertexType, typename IndexType /*= uint32_t*/>
void MeshData<VertexType, IndexType>::createVertexBuffer(const VulkanCore* vulkanCore)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkanCore->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vulkanCore->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(vulkanCore->GetDevice(), stagingBufferMemory);

    vulkanCore->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    vulkanCore->copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(vulkanCore->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanCore->GetDevice(), stagingBufferMemory, nullptr);
}

template <typename VertexType, typename IndexType /*= uint32_t*/>
bool MeshData<VertexType, IndexType>::releaseModelFromGPU()
{
    if (!isOnGPU)
        return false;
    assert(m_pVulkanCore != nullptr);

    if (indexBuffer.has_value())
        vkDestroyBuffer(m_pVulkanCore->GetDevice(), indexBuffer.value(), nullptr);
    if (indexBufferMemory.has_value())
        vkFreeMemory(m_pVulkanCore->GetDevice(), indexBufferMemory.value(), nullptr);

    vkDestroyBuffer(m_pVulkanCore->GetDevice(), vertexBuffer, nullptr);
    vkFreeMemory(m_pVulkanCore->GetDevice(), vertexBufferMemory, nullptr);
    return true;
}

template <typename VertexType, typename IndexType /*= uint32_t*/>
bool MeshData<VertexType, IndexType>::uploadModelToGPU(const VulkanCore* vulkanCore)
{
    if (isOnGPU)
        return true;
    createVertexBuffer(vulkanCore);

    if (indices.has_value())
        createIndexBuffer(vulkanCore);

    isOnGPU = true;
    m_pVulkanCore = vulkanCore;
    return true;
}