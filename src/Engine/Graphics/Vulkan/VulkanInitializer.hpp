#pragma once

#include "pch.hpp"
#include "VulkanCore.hpp"
#include "VulkanHelper.hpp"

void createCommandBuffer(const VkDevice& device, const VkCommandPool & commandPool, VkCommandBuffer& commandBuffer)
{
    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));
}

void createDescriptorSet(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorPool& descriptorPool, VkDescriptorSet& descriptorSet)
{
    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout,
    };
    VK(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));
}
