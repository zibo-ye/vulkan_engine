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

std::vector<char> readShaderFile(const std::filesystem::path& filename)
{
	printf("Reading file: %s\n", filename.string().c_str());
	// Check if the file exists and is not a directory
	if (!std::filesystem::exists(filename)) {
		printf("File not found!\n");
		throw std::runtime_error("File not found!");
	}
	if (std::filesystem::is_directory(filename)) {
		printf("File is a directory!\n");
		throw std::runtime_error("File is a directory!");
	}

	std::ifstream file(filename.string(), std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
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

void printAllAvailableDeviceExtensions(std::string deviceName)
{
    VulkanCore core;
    core.createInstance();

    std::vector<VkPhysicalDevice> pdevices = getAllPhysicalDevices(core.GetInstance());

    for (const auto& pdevice : pdevices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(pdevice, &properties);
        if (std::string(properties.deviceName) == deviceName) {
            printAllAvailableDeviceExtensions(pdevice);
            return;
        }
    }
    std::cout << "Can't find device with name: " << deviceName << std::endl;
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

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* pUserData)
{
    // https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf
    std::string prefix = "";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        prefix = "VERBOSE : ";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        prefix = "INFO : ";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        prefix = "WARNING : ";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        prefix = "ERROR : ";
    }
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        prefix += "GENERAL";
    } else {
        if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
            prefix += "PERF";
        }
    }

    std::string message = prefix + " - Message ID Number " + std::to_string(callbackData->messageIdNumber) + ", Message ID String " + callbackData->pMessageIdName + ":\n" + callbackData->pMessage;
    if (callbackData->objectCount > 0) {
        message += "\n Objects - " + std::to_string(callbackData->objectCount) + "\n";
        for (uint32_t object = 0; object < callbackData->objectCount; ++object) {
            std::string objectMessage = " Object[" + std::to_string(object) + "] - Type " + DebugAnnotObjectToString(callbackData->pObjects[object].objectType) + ", Value " + std::to_string(reinterpret_cast<uint64_t>(callbackData->pObjects[object].objectHandle)) + ", Name \"" + callbackData->pObjects[object].pObjectName + "\"\n";
            message += objectMessage;
        }
    }
    if (callbackData->cmdBufLabelCount > 0) {
        message += "\n Command Buffer Labels - " + std::to_string(callbackData->cmdBufLabelCount) + "\n";
        for (uint32_t label = 0; label < callbackData->cmdBufLabelCount; ++label) {
            std::string labelMessage = " Label[" + std::to_string(label) + "] - " + callbackData->pCmdBufLabels[label].pLabelName + " { " + std::to_string(callbackData->pCmdBufLabels[label].color[0]) + ", " + std::to_string(callbackData->pCmdBufLabels[label].color[1]) + ", " + std::to_string(callbackData->pCmdBufLabels[label].color[2]) + ", " + std::to_string(callbackData->pCmdBufLabels[label].color[3]) + "}\n";
            message += labelMessage;
        }
    }
    std::cout << message << std::endl;
    return VK_FALSE;
}

const char* DebugAnnotObjectToString(VkObjectType t)
{
    switch (t) {
    case VK_OBJECT_TYPE_UNKNOWN:
        return "VK_OBJECT_TYPE_UNKNOWN";
    case VK_OBJECT_TYPE_INSTANCE:
        return "VK_OBJECT_TYPE_INSTANCE";
    case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
        return "VK_OBJECT_TYPE_PHYSICAL_DEVICE";
    case VK_OBJECT_TYPE_DEVICE:
        return "VK_OBJECT_TYPE_DEVICE";
    case VK_OBJECT_TYPE_QUEUE:
        return "VK_OBJECT_TYPE_QUEUE";
    case VK_OBJECT_TYPE_SEMAPHORE:
        return "VK_OBJECT_TYPE_SEMAPHORE";
    case VK_OBJECT_TYPE_COMMAND_BUFFER:
        return "VK_OBJECT_TYPE_COMMAND_BUFFER";
    case VK_OBJECT_TYPE_FENCE:
        return "VK_OBJECT_TYPE_FENCE";
    case VK_OBJECT_TYPE_DEVICE_MEMORY:
        return "VK_OBJECT_TYPE_DEVICE_MEMORY";
    case VK_OBJECT_TYPE_BUFFER:
        return "VK_OBJECT_TYPE_BUFFER";
    case VK_OBJECT_TYPE_IMAGE:
        return "VK_OBJECT_TYPE_IMAGE";
    case VK_OBJECT_TYPE_EVENT:
        return "VK_OBJECT_TYPE_EVENT";
    case VK_OBJECT_TYPE_QUERY_POOL:
        return "VK_OBJECT_TYPE_QUERY_POOL";
    case VK_OBJECT_TYPE_BUFFER_VIEW:
        return "VK_OBJECT_TYPE_BUFFER_VIEW";
    case VK_OBJECT_TYPE_IMAGE_VIEW:
        return "VK_OBJECT_TYPE_IMAGE_VIEW";
    case VK_OBJECT_TYPE_SHADER_MODULE:
        return "VK_OBJECT_TYPE_SHADER_MODULE";
    case VK_OBJECT_TYPE_PIPELINE_CACHE:
        return "VK_OBJECT_TYPE_PIPELINE_CACHE";
    case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
        return "VK_OBJECT_TYPE_PIPELINE_LAYOUT";
    case VK_OBJECT_TYPE_RENDER_PASS:
        return "VK_OBJECT_TYPE_RENDER_PASS";
    case VK_OBJECT_TYPE_PIPELINE:
        return "VK_OBJECT_TYPE_PIPELINE";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
        return "VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT";
    case VK_OBJECT_TYPE_SAMPLER:
        return "VK_OBJECT_TYPE_SAMPLER";
    case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
        return "VK_OBJECT_TYPE_DESCRIPTOR_POOL";
    case VK_OBJECT_TYPE_DESCRIPTOR_SET:
        return "VK_OBJECT_TYPE_DESCRIPTOR_SET";
    case VK_OBJECT_TYPE_FRAMEBUFFER:
        return "VK_OBJECT_TYPE_FRAMEBUFFER";
    case VK_OBJECT_TYPE_COMMAND_POOL:
        return "VK_OBJECT_TYPE_COMMAND_POOL";
    case VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION:
        return "VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION";
    case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
        return "VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE";
    case VK_OBJECT_TYPE_SURFACE_KHR:
        return "VK_OBJECT_TYPE_SURFACE_KHR";
    case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
        return "VK_OBJECT_TYPE_SWAPCHAIN_KHR";
    case VK_OBJECT_TYPE_DISPLAY_KHR:
        return "VK_OBJECT_TYPE_DISPLAY_KHR";
    case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
        return "VK_OBJECT_TYPE_DISPLAY_MODE_KHR";
    case VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT:
        return "VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT";
#ifdef VK_NVX_device_generated_commands
    case VK_OBJECT_TYPE_OBJECT_TABLE_NVX:
        return "VK_OBJECT_TYPE_OBJECT_TABLE_NVX";
    case VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX:
        return "VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX";
#endif
    case VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT:
        return "VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT";
    case VK_OBJECT_TYPE_VALIDATION_CACHE_EXT:
        return "VK_OBJECT_TYPE_VALIDATION_CACHE_EXT";
#ifdef VK_NV_ray_tracing
    case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV:
        return "VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV";
#endif
        //	case VK_OBJECT_TYPE_RANGE_SIZE:
    case VK_OBJECT_TYPE_MAX_ENUM:
        break;
    default:
        break;
    }
    return "UNKNOWNTYPE";
}