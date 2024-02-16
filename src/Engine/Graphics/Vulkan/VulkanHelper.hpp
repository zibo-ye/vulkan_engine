#pragma once

#include "pch.hpp"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

#ifndef NDEBUG
#define VK_CHECK_RESULT(FN)                                                                                                                     \
    {                                                                                                                                           \
        if (VkResult result = FN) {                                                                                                             \
            throw std::runtime_error("Call '" #FN "' returned " + std::to_string(result) + " [" + std::string(string_VkResult(result)) + "]."); \
        }                                                                                                                                       \
    }
#else
#define VK_CHECK_RESULT(FN) FN
#endif

std::vector<VkExtensionProperties> getAllAvailableInstanceExtensions();
std::vector<VkLayerProperties> getAllAvailableLayers();
std::vector<VkExtensionProperties> getAllAvailableDeviceExtensions(VkPhysicalDevice device);
std::vector<VkPhysicalDevice> getAllPhysicalDevices(VkInstance instance);
std::vector<VkQueueFamilyProperties> getAllQueueFamilies(VkPhysicalDevice device);
VkPhysicalDeviceMemoryProperties getAllMemoryProperties(VkPhysicalDevice device);
VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
bool hasStencilComponent(VkFormat format);

void printAllPhysicalDevices();
void printAllAvailableInstanceExtensions();
void printAllAvailableLayers();
void printAllAvailableDeviceExtensions(VkPhysicalDevice device);
void printAllQueueFamilies(VkPhysicalDevice device, std::vector<VkQueueFamilyProperties> queueFamilies);
void printMemoryTypeProperties(VkMemoryPropertyFlags flags);
void printMemoryHeapProperties(VkMemoryHeapFlags flags);

void printAllMemoryProperties(VkPhysicalDeviceMemoryProperties& memoryProperties);
void printAllMemoryTypeProperties(VkMemoryPropertyFlags flags);
void printAllMemoryHeapProperties(VkMemoryHeapFlags flags);
void printAllPhysicalDevices(std::vector<VkPhysicalDevice> physicalDevices);
