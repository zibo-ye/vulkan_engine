
#include "Model.hpp"
#include "VulkanCore.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "ThirdParty/tiny_obj_loader.h"

bool Model::loadModelFromFile(std::string path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices {};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex {
                .pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2] },
                .color = { 1.0f, 1.0f, 1.0f },
                .texCoord = { attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1] },
            };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    return true;
}

bool Model::uploadModelToGPU(const VulkanCore* vulkanCore)
{
    if (isOnGPU)
        return false;
    createVertexBuffer(vulkanCore);
    createIndexBuffer(vulkanCore);

    isOnGPU = true;
    m_pVulkanCore = vulkanCore;
    return true;
}

bool Model::releaseModelFromGPU()
{
    if (!isOnGPU)
        return false;
    assert(m_pVulkanCore != nullptr);
    vkDestroyBuffer(m_pVulkanCore->GetDevice(), indexBuffer, nullptr);
    vkFreeMemory(m_pVulkanCore->GetDevice(), indexBufferMemory, nullptr);

    vkDestroyBuffer(m_pVulkanCore->GetDevice(), vertexBuffer, nullptr);
    vkFreeMemory(m_pVulkanCore->GetDevice(), vertexBufferMemory, nullptr);
    return true;
}


void Model::createVertexBuffer(const VulkanCore* vulkanCore)
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

void Model::createIndexBuffer(const VulkanCore* vulkanCore)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkanCore->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vulkanCore->GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(vulkanCore->GetDevice(), stagingBufferMemory);

    vulkanCore->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    vulkanCore->copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(vulkanCore->GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanCore->GetDevice(), stagingBufferMemory, nullptr);
}