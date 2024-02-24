#pragma once

#include "Image.hpp"
#include "VulkanHelper.hpp"
#include "Window/IWindow.hpp"
#include "pch.hpp"

class Scene;
namespace EngineCore {
class IApp;
}

class Image;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensionsWithoutSwapchain = {
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct UniformBufferObject {
    alignas(16) vkm::mat4 view;
    alignas(16) vkm::mat4 proj;
};

struct FrameData {
    VkCommandBuffer commandBuffer;
    VkDescriptorSet descriptorSet;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence swapchainImageFence;
    DeletionQueue deletionQueue;

    void Destroy(const VkDevice& device);
};

class VulkanCore {
public:
    void Init(EngineCore::IApp* pApp);
    void Shutdown();
    void drawFrame(Scene& scene);

private:
    EngineCore::IApp* m_pApp = nullptr;

public: // High level
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void WaitIdle();

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::optional<VkSurfaceKHR> surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

private: // Swapchain
    void createSwapChain();
    void createSwapchainImageViews();
    void cleanupSwapChain();
    void recreateSwapChain();

    VkSwapchainKHR swapChain;
    std::vector<Image> swapChainImages;

private:
    void createFrameData();
    void createFrameSyncObjects(VkSemaphore& imageAvailableSemaphore, VkSemaphore& renderFinishedSemaphore, VkFence& swapchainImageFence);

    std::vector<FrameData> frames;
    uint32_t currentFrameInFlight = 0;

private:
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;

    Image depthImage;
    Image colorImage;

    DeletionQueue mainDeletionQueue;

private:
    void createDescriptorSetLayout();
    void createDescriptorPool();

    void createGraphicsPipeline();

    void createCommandPool();

    void createColorResources();

    void createDepthResources();

    VkFormat findDepthFormat();

    std::unique_ptr<uint8_t[]> copyTextureToMemory(Image textureImage, uint32_t texWidth, uint32_t texHeight);

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    void createUniformBuffers();

    void InitializeDescriptorSets();
    void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene& scene);

    void updateUniformBuffer(uint32_t currentImage);

public:
    // #TODO: Buffer abstraction
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void copyImageToBuffer(Image image, VkBuffer buffer, uint32_t width, uint32_t height);

public: // Helper
    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkShaderModule createShaderModule(const std::vector<char>& code);

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    std::vector<const char*> getRequiredExtensions();

    bool checkValidationLayerSupport();

    static std::vector<char> readFile(const std::filesystem::path& filename);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

public:
    VkDevice GetDevice() const { return device; }
    VkInstance GetInstance() const { return instance; }
    bool IsHeadless() const { return (m_pApp && m_pApp->info.window->IsHeadless()); }

public:
    void PresentImage();
    void SaveFrame(const std::string& savePath);
    bool readyForNextImage = false;

private:
    uint32_t AcquireNextImageIndex();
    uint32_t nextImageIndex = 0;
};
