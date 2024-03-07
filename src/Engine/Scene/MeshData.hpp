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
};

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