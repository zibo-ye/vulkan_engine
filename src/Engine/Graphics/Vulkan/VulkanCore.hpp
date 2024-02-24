#pragma once

#include "Window/IWindow.hpp"
#include "pch.hpp"

class Scene;
namespace EngineCore {
class IApp;
}

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensionsWithoutSwapchain = {};
const std::vector<const char*> deviceExtensions = {
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

    void Destroy(const VkDevice& device);
};

class VulkanCore {
private:
    EngineCore::IApp* m_pApp = nullptr;

private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    std::optional<VkSurfaceKHR> surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;

    std::vector<VkDeviceMemory> swapChainImagesMemory;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;

    std::vector<FrameData> frames;
    uint32_t currentFrameInFlight = 0;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

public:
    void Init(EngineCore::IApp* pApp);

    void cleanupSwapChain();

    void WaitIdle();

    void Shutdown();

    void recreateSwapChain();

    void createInstance();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    VkSampleCountFlagBits getMaxUsableSampleCount();

    void createLogicalDevice();

    void createSwapChain();

    void createSwapchainImageViews();

    void createRenderPass();

    void createDescriptorSetLayout();

    void createGraphicsPipeline();

    void createFramebuffers();

    void createCommandPool();

    void createColorResources();

    void createDepthResources();

    VkFormat findDepthFormat();

    std::unique_ptr<uint8_t[]> copyTextureToMemory(VkImage textureImage, uint32_t texWidth, uint32_t texHeight);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    void createUniformBuffers();

    void createDescriptorPool();

    void createFrameData();
    void createFrameSyncObjects(VkSemaphore& imageAvailableSemaphore, VkSemaphore& renderFinishedSemaphore, VkFence& swapchainImageFence);

    void InitializeDescriptorSets();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void copyImageToBuffer(VkImage image, VkBuffer buffer, uint32_t width, uint32_t height);
    VkCommandBuffer beginSingleTimeCommands() const;

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

    void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene& scene);

    void updateUniformBuffer(uint32_t currentImage);

    void drawFrame(Scene& scene);

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
    uint32_t AcquireLastImageIndex();

    uint32_t nextImageIndex = 0;
    uint32_t lastImageIndex = 0;
};
