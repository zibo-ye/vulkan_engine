#pragma once

#include "VulkanCore.hpp"
#include "VulkanHelper.hpp"
#include "pch.hpp"

void createCommandBuffer(const VkDevice& device, const VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);

void createDescriptorSet(const VkDevice& device, const VkDescriptorSetLayout& descriptorSetLayout, const VkDescriptorPool& descriptorPool, VkDescriptorSet& descriptorSet);
