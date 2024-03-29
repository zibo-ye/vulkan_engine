#pragma once
#include "Graphics/Vulkan/VulkanCore.hpp"
#include "pch.hpp"

template <typename VertexType, typename IndexType = uint32_t>
class MeshData {
public:
    MeshData() = default;
    ~MeshData()
    {
        // assert(!isOnGPU && "MeshData is still on GPU. Should call releaseModelFromGPU() before destroying the object.")
    }
    std::vector<VertexType> vertices;
    std::optional<std::vector<IndexType>> indices;

public:
    bool uploadModelToGPU(VulkanCore* vulkanCore);
    bool releaseModelFromGPU();

    Buffer vertexBuffer;
    std::optional<Buffer> indexBuffer;
    // #TODO: buffer's lifecycle is limited by vulkanCore.
private:
    void createVertexBuffer();
    void createIndexBuffer();

private:
    bool isOnGPU = false;
    VulkanCore* m_pVulkanCore = nullptr;

public:
    void draw(VkCommandBuffer commandBuffer);
};

template <typename VertexType, typename IndexType /*= uint32_t*/>
void MeshData<VertexType, IndexType>::draw(VkCommandBuffer commandBuffer)
{
    if (!isOnGPU)
        throw std::runtime_error("MeshData is not on GPU. Call uploadModelToGPU() before drawing.");

    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);

    if (indexBuffer.has_value()) {
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.value().buffer, 0, sizeof(IndexType) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.value().size()), 1, 0, 0, 0);
    } else {
        vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    }
}

template <typename VertexType, typename IndexType /*= uint32_t*/>
void MeshData<VertexType, IndexType>::createIndexBuffer()
{
    if (!indices.has_value())
        return;
    VkDeviceSize bufferSize = sizeof(indices.value()[0]) * indices.value().size();

    indexBuffer = Buffer();
    indexBuffer->Init(m_pVulkanCore, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    indexBuffer->UploadData(indices.value().data(), (size_t)bufferSize);
}

template <typename VertexType, typename IndexType /*= uint32_t*/>
void MeshData<VertexType, IndexType>::createVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    vertexBuffer.Init(m_pVulkanCore, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vertexBuffer.UploadData(vertices.data(), (size_t)bufferSize);
}

template <typename VertexType, typename IndexType /*= uint32_t*/>
bool MeshData<VertexType, IndexType>::releaseModelFromGPU()
{
    if (!isOnGPU)
        return false;
    assert(m_pVulkanCore != nullptr);

    auto device = m_pVulkanCore->GetDevice();
    if (device != VK_NULL_HANDLE) {
        if (indexBuffer)
            indexBuffer->Destroy();
        vertexBuffer.Destroy();
    }
    isOnGPU = false;
    return true;
}

template <typename VertexType, typename IndexType /*= uint32_t*/>
bool MeshData<VertexType, IndexType>::uploadModelToGPU(VulkanCore* vulkanCore)
{
    if (isOnGPU)
        return true;
    m_pVulkanCore = vulkanCore;
    createVertexBuffer();

    if (indices.has_value())
        createIndexBuffer();

    m_pVulkanCore->mainDeletionStack.push([=]() {
        releaseModelFromGPU();
    });
    isOnGPU = true;
    return true;
}