#include "VulkanHelper.hpp"
#include "VulkanCore.hpp"

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

std::vector<VkExtensionProperties> getAllAvailableInstanceExtensions()
{
    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
}

std::vector<VkLayerProperties> getAllAvailableLayers()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return availableLayers;
}

std::vector<VkExtensionProperties> getAllAvailableDeviceExtensions(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
}

std::vector<VkPhysicalDevice> getAllPhysicalDevices(VkInstance instance)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    return devices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, std::optional<VkSurfaceKHR> surface)
{
    QueueFamilyIndices indices;
    std::vector<VkQueueFamilyProperties> queueFamilies = getAllQueueFamilies(device);

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (surface.has_value()) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface.value(), &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }
        }

        if (indices.isComplete(!surface.has_value())) {
            break;
        }

        i++;
    }

    return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device, std::optional<VkSurfaceKHR> surface)
{
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    bool extensionsSupported = checkDeviceExtensionSupport(device, !surface.has_value());

    if (!surface.has_value()) // Headless
    {
        return indices.isComplete(!surface.has_value()) && extensionsSupported && supportedFeatures.samplerAnisotropy;
    } else {
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface.value());
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        return indices.isComplete(!surface.has_value()) && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, bool isHeadless)
{
    std::vector<VkExtensionProperties> availableExtensions = getAllAvailableDeviceExtensions(device);
    std::set<std::string> requiredExtensions;
    if (isHeadless) {
        requiredExtensions = std::set<std::string>(deviceExtensionsWithoutSwapchain.begin(), deviceExtensionsWithoutSwapchain.end());
    } else {
        requiredExtensions = std::set<std::string>(deviceExtensions.begin(), deviceExtensions.end());
    }

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

std::vector<VkQueueFamilyProperties> getAllQueueFamilies(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    return queueFamilies;
}

VkPhysicalDeviceMemoryProperties getAllMemoryProperties(VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

    return memProperties;
}

VkFormat findSupportedFormat(VkPhysicalDevice device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void printAllPhysicalDevices()
{
    VulkanCore core;
    core.createInstance();

    auto devices = getAllPhysicalDevices(core.GetInstance());
    std::cout << "Available Physical Devices:\n";
    int idx = 0;
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(device, &properties);
        std::cout << "Device " << idx++ << ": Name:\t" << properties.deviceName << '\n';
        std::cout << std::hex << "\tapiVersion:\t0x" << properties.apiVersion << '\n';
        std::cout << std::hex << "\tdriverVersion:\t0x" << properties.driverVersion << '\n';
        std::cout << std::hex << "\tvendorID:\t0x" << properties.vendorID << '\n';
        std::cout << std::hex << "\tdeviceID:\t0x" << properties.deviceID << std::dec << "\n\n";
        // std::cout << '\t' << features << '\n';
    }
}

void printAllAvailableInstanceExtensions()
{
    std::vector<VkExtensionProperties> availableExtensions = getAllAvailableInstanceExtensions();
    std::cout << "available instance extensions:\n";
    for (const auto& extension : availableExtensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

void printAllAvailableLayers()
{
    std::vector<VkLayerProperties> availableLayers = getAllAvailableLayers();
    std::cout << "available layers:\n";
    for (const auto& layer : availableLayers) {
        std::cout << '\t' << layer.layerName << '\n';
    }
}

void printAllAvailableDeviceExtensions(VkPhysicalDevice device)
{
    std::vector<VkExtensionProperties> availableExtensions = getAllAvailableDeviceExtensions(device);
    std::cout << "Available Device Extensions:\n";
    for (const auto& extension : availableExtensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

void printAllQueueFamilies(VkPhysicalDevice device, std::vector<VkQueueFamilyProperties> queueFamilies)
{
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(device, &properties);
    std::cout << "Device Name:\t" << properties.deviceName << '\n';

    std::cout << "Available Queue Families:\n";
    for (const auto& queueFamily : queueFamilies) {
        std::cout << "\tqueueCount:\t" << queueFamily.queueCount << '\n';
        std::cout << "\tqueueFlags:\t" << std::hex << queueFamily.queueFlags << std::dec << '\n';

        uint32_t flags = queueFamily.queueFlags;
        uint32_t bitPosition = 1;
        while (flags) {
            if (flags & 1) { // Check if the LSB is set
                std::string flagName = string_VkQueueFlagBits(static_cast<VkQueueFlagBits>(bitPosition));
                if (!flagName.empty()) {
                    std::cout << "\t\t" << flagName << "\n";
                }
            }
            flags >>= 1; // Right shift the flags to check the next bit in the next iteration
            bitPosition <<= 1;
        }

        std::cout << "\ttimestampValidBits:\t" << queueFamily.timestampValidBits << "\n\n";
    }
}

void printMemoryTypeProperties(VkMemoryPropertyFlags flags)
{
    std::cout << "\t\tMemory Properties: ";
    if (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        std::cout << "DEVICE_LOCAL ";
    if (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        std::cout << "HOST_VISIBLE ";
    if (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
        std::cout << "HOST_COHERENT ";
    if (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
        std::cout << "HOST_CACHED ";
    if (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
        std::cout << "LAZILY_ALLOCATED ";
    if (flags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
        std::cout << "PROTECTED ";
    if (flags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
        std::cout << "DEVICE_COHERENT_AMD ";
    if (flags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
        std::cout << "DEVICE_UNCACHED_AMD ";
    if (flags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
        std::cout << "RDMA_CAPABLE_NV ";
    std::cout << std::endl;
}

void printMemoryHeapProperties(VkMemoryHeapFlags flags)
{
    std::cout << "\t\tHeap Properties: ";
    if (flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
        std::cout << "DEVICE_LOCAL ";
    if (flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT)
        std::cout << "MULTI_INSTANCE ";
    std::cout << std::endl;
}

void printAllMemoryProperties(VkPhysicalDeviceMemoryProperties& memProperties)
{
    std::cout << "Available Memory Types:\n";
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        const auto& memType = memProperties.memoryTypes[i];
        std::cout << "\tMemory Type " << i << ":\n";
        printMemoryTypeProperties(memType.propertyFlags);
        std::cout << "\t\tHeap Index: " << memType.heapIndex << std::endl;
    }

    std::cout << "Memory Heaps:\n";
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; ++i) {
        const auto& memHeap = memProperties.memoryHeaps[i];
        std::cout << "\tMemory Heap " << i << ":\n";
        std::cout << "\t\tSize: " << std::hex << memHeap.size << std::dec << std::endl;
        printMemoryHeapProperties(memHeap.flags);
        std::cout << std::endl;
    }
}