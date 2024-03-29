#pragma once

#include "pch.hpp"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

#ifndef NDEBUG
#define VK(FN)                                                                                                                                  \
    {                                                                                                                                           \
        if (VkResult result = FN) {                                                                                                             \
            std::cerr << "Call '" #FN "' returned " << result << " [" << string_VkResult(result) << "]." << std::endl;                          \
            throw std::runtime_error("Call '" #FN "' returned " + std::to_string(result) + " [" + std::string(string_VkResult(result)) + "]."); \
        }                                                                                                                                       \
    }
#else
#define VK(FN) FN
#endif

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete(bool headless)
    {
        if (headless) {
            return graphicsFamily.has_value();
        }
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    std::vector<uint32_t> getAllIndices()
    {
        std::vector<uint32_t> indices;
        if (graphicsFamily.has_value()) {
            indices.push_back(graphicsFamily.value());
        }
        if (presentFamily.has_value()) {
            indices.push_back(presentFamily.value());
        }
        return indices;
    }

    std::set<uint32_t> getUniqueIndices()
    {
        std::set<uint32_t> indices;
        if (graphicsFamily.has_value()) {
            indices.insert(graphicsFamily.value());
        }
        if (presentFamily.has_value()) {
            indices.insert(presentFamily.value());
        }
        return indices;
    }
};

// A easier way to handle resource destruction
struct DeletionStack {
    std::stack<std::function<void()>> deletors;

    void push(std::function<void()>&& func)
    {
        deletors.push(func);
    }

    void flush()
    {
        while (!deletors.empty()) {
            deletors.top()();
            deletors.pop();
        }
    }
};

std::vector<VkExtensionProperties> getAllAvailableInstanceExtensions();
std::vector<VkLayerProperties> getAllAvailableLayers();
std::vector<VkExtensionProperties> getAllAvailableDeviceExtensions(VkPhysicalDevice device);
std::vector<VkPhysicalDevice> getAllPhysicalDevices(VkInstance instance);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, std::optional<VkSurfaceKHR> surface);
bool isDeviceSuitable(VkPhysicalDevice device, std::optional<VkSurfaceKHR> surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device, bool isHeadless);
std::vector<VkQueueFamilyProperties> getAllQueueFamilies(VkPhysicalDevice device);
VkPhysicalDeviceMemoryProperties getAllMemoryProperties(VkPhysicalDevice device);
VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
bool hasStencilComponent(VkFormat format);

std::vector<char> readShaderFile(const std::filesystem::path& filename);


void printAllPhysicalDevices();
void printAllAvailableInstanceExtensions();
void printAllAvailableLayers();
void printAllAvailableDeviceExtensions(std::string deviceName);
void printAllAvailableDeviceExtensions(VkPhysicalDevice device);
void printAllQueueFamilies(VkPhysicalDevice device, std::vector<VkQueueFamilyProperties> queueFamilies);
void printMemoryTypeProperties(VkMemoryPropertyFlags flags);
void printMemoryHeapProperties(VkMemoryHeapFlags flags);

void printAllMemoryProperties(VkPhysicalDeviceMemoryProperties& memoryProperties);
void printAllMemoryTypeProperties(VkMemoryPropertyFlags flags);
void printAllMemoryHeapProperties(VkMemoryHeapFlags flags);
void printAllPhysicalDevices(std::vector<VkPhysicalDevice> physicalDevices);

extern VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* pUserData);
extern const char* DebugAnnotObjectToString(VkObjectType t);
