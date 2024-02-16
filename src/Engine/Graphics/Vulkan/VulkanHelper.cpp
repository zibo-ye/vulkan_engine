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

void printAllAvailableLayers()
{
    std::vector<VkLayerProperties> availableLayers = getAllAvailableLayers();
    std::cout << "available layers:\n";
    for (const auto& layer : availableLayers) {
        std::cout << '\t' << layer.layerName << '\n';
    }
}

std::vector<VkLayerProperties> getAllAvailableLayers()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return availableLayers;
}

void printAllAvailableDeviceExtensions(VkPhysicalDevice device)
{
    std::vector<VkExtensionProperties> availableExtensions = getAllAvailableDeviceExtensions(device);
    std::cout << "Available Device Extensions:\n";
    for (const auto& extension : availableExtensions) {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

std::vector<VkExtensionProperties> getAllAvailableDeviceExtensions(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    return availableExtensions;
}

std::vector<VkPhysicalDevice> listAllPhysicalDevices(VkInstance instance)
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

std::vector<VkQueueFamilyProperties> listAllQueueFamilies(VkPhysicalDevice device)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#if VERBOSE

    std::map<VkQueueFlagBits, std::string> flagNames = {
        { VK_QUEUE_GRAPHICS_BIT, "VK_QUEUE_GRAPHICS_BIT" },
        { VK_QUEUE_COMPUTE_BIT, "VK_QUEUE_COMPUTE_BIT" },
        { VK_QUEUE_TRANSFER_BIT, "VK_QUEUE_TRANSFER_BIT" },
        { VK_QUEUE_SPARSE_BINDING_BIT, "VK_QUEUE_SPARSE_BINDING_BIT" },
        { VK_QUEUE_PROTECTED_BIT, "VK_QUEUE_PROTECTED_BIT" },
        { VK_QUEUE_VIDEO_DECODE_BIT_KHR, "VK_QUEUE_VIDEO_DECODE_BIT_KHR" },
        //{ VK_QUEUE_VIDEO_ENCODE_BIT_KHR, "VK_QUEUE_VIDEO_ENCODE_BIT_KHR" },
        { VK_QUEUE_OPTICAL_FLOW_BIT_NV, "VK_QUEUE_OPTICAL_FLOW_BIT_NV" }
    };
    static std::set<VkPhysicalDevice> printedDevices;
    if (printedDevices.find(device) == printedDevices.end()) {
        std::cout << "Available Queue Families:\n";
        for (const auto& queueFamily : queueFamilies) {
            std::cout << "\t\tqueueCount:\t" << queueFamily.queueCount << '\n';
            std::cout << "\t\tqueueFlags:\t" << queueFamily.queueFlags << '\n';
            for (const auto& flag : flagNames) {
                if (queueFamily.queueFlags & flag.first) {
                    std::cout << "\t\t\t" << flag.second << "\n";
                }
            }
            std::cout << "\t\ttimestampValidBits:\t" << queueFamily.timestampValidBits << "\n\n";
            // std::cout << '\t' << queueFamily.minImageTransferGranularity << '\n';
        }
        printedDevices.insert(device);
    }

#endif // VERBOSE

    return queueFamilies;
}

VkPhysicalDeviceMemoryProperties getAllMemoryProperties(VkPhysicalDevice device)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

#if VERBOSE
    static std::set<VkPhysicalDevice> printedDevices;
    if (printedDevices.find(device) == printedDevices.end()) {
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
        printedDevices.insert(device);
    }
#endif // VERBOSE
    return memProperties;
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

void printPhysicalDevices()
{
    VulkanCore core;
    core.createInstance();

    auto devices = listAllPhysicalDevices(core.GetInstance());
    std::cout << "Available Physical Devices:\n";
    int idx = 0;
    for (const auto& device : devices) {
        VkPhysicalDeviceProperties properties {};
        vkGetPhysicalDeviceProperties(device, &properties);
        std::cout << "Device " << idx++ << ":\n";
        std::cout << std::hex << "\tDevice Name:\t0x" << properties.deviceName << '\n';
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
