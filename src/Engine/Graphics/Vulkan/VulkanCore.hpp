#pragma once

#include "Buffer.hpp"
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
const int MAX_MATERIAL_TYPES = 1024;
const int MAX_DESCRIPTORS_IN_MATERIAL = 4;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensionsWithoutSwapchain = {
#ifdef __APPLE__
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
};
const std::vector<const char*> deviceExtensions = {
#ifdef __APPLE__
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct CameraUBO {
    alignas(16) vkm::mat4 view;
    alignas(16) vkm::mat4 proj;
    alignas(16) vkm::mat4 viewproj;
    alignas(4) vkm::vec4 position;
};

struct FrameData {
    VkCommandBuffer commandBuffer;
    VkDescriptorSet descriptorSet;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence swapchainImageFence;
    DeletionStack deletionStack;
    Buffer uniformBuffer;

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
    DeletionStack mainDeletionStack;

private: // Swapchain
    void createSwapChain();
    void createSwapchainImageViews();
    void cleanupSwapChain();
    void recreateSwapChain();

    VkSwapchainKHR swapChain;
    std::vector<Image> swapChainImages;

private: // Frame
    void createFrameData();
    void createFrameSyncObjects(VkSemaphore& imageAvailableSemaphore, VkSemaphore& renderFinishedSemaphore, VkFence& swapchainImageFence);

    std::vector<FrameData> frames;
    uint32_t currentFrameInFlight = 0;

public:
    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts; // TEMP public, [1] is for material
    VkDescriptorPool descriptorPool;

private:
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    VkCommandPool commandPool;

    Image depthImage;
    Image colorImage;

private:
    void createDescriptorSetLayout();
    void createDescriptorPool();

    void createGraphicsPipeline();

    void createCommandPool();

    void createColorResources();

    void createDepthResources();

    void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene& scene);

    void createUniformBuffers(Buffer& uniformBuffer);

    void updateUniformBuffer(uint32_t currentImage);

    void updateDescriptorSet(uint32_t currentFrameInFlight, Scene& scene);

public: // Helper
    struct SPushConstant {
        vkm::mat4 matWorld;
        vkm::mat4 matNormal;
    };

    VkFormat findDepthFormat();
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

public:
    VkDevice GetDevice() const { return device; }
    VkPhysicalDevice GetPhysicalDevice() const { return physicalDevice; }
    VkInstance GetInstance() const { return instance; }

public:
    bool IsHeadless() const { return (m_pApp && m_pApp->info.window->IsHeadless()); }
    void PresentImage();
    void SaveFrame(const std::string& savePath);
    bool readyForNextImage = false;

private:
    uint32_t AcquireNextImageIndex();
    uint32_t nextImageIndex = 0;
};
