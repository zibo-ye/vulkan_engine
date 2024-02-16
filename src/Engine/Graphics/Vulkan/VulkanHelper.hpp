#pragma once

#include "pch.hpp"

#ifndef NDEBUG
#define VK_CHECK_RESULT(FN)                                                                                                                \
	{                                                                                                                                      \
		if (VkResult result = FN) {                                                                                                         \
			throw std::runtime_error("Call '" #FN "' returned " + std::to_string(result) + " [" + std::string(string_VkResult(result)) + "]."); \
		}                                                                                                                                  \
	}
#else
#define VK_CHECK_RESULT(FN) FN
#endif

void printPhysicalDevices();
void printAllAvailableInstanceExtensions();
void printAllAvailableLayers();
void printAllAvailableDeviceExtensions(VkPhysicalDevice device);
void printAllQueueFamilies(std::vector<VkQueueFamilyProperties> queueFamilies);
void printAllMemoryProperties(VkPhysicalDeviceMemoryProperties memoryProperties);
void printAllMemoryTypeProperties(VkMemoryPropertyFlags flags);
void printAllMemoryHeapProperties(VkMemoryHeapFlags flags);
void printAllPhysicalDevices(std::vector<VkPhysicalDevice> physicalDevices);

std::vector<VkExtensionProperties> getAllAvailableInstanceExtensions();
std::vector<VkLayerProperties> getAllAvailableLayers();
std::vector<VkExtensionProperties> getAllAvailableDeviceExtensions(VkPhysicalDevice device);
std::vector<VkPhysicalDevice> listAllPhysicalDevices(VkInstance instance);
std::vector<VkQueueFamilyProperties> listAllQueueFamilies(VkPhysicalDevice device);
VkPhysicalDeviceMemoryProperties getAllMemoryProperties(VkPhysicalDevice device);
void printMemoryTypeProperties(VkMemoryPropertyFlags flags);
void printMemoryHeapProperties(VkMemoryHeapFlags flags);
VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
bool hasStencilComponent(VkFormat format);
