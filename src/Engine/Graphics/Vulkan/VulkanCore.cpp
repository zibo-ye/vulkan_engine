

#define _CRT_SECURE_NO_WARNINGS
#include "VulkanCore.hpp"
#include "Shader.hpp"

#include "EngineCore.hpp"
#include "Scene/CameraManager.hpp"
#include "Scene/Environment.hpp"
#include "Scene/Material.hpp"
#include "Scene/Mesh.hpp"
#include "Scene/Scene.hpp"
#include "VulkanInitializer.hpp"
#include "../Main/main.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"

#if USE_GLM
namespace vkm {
void test_vkm_glm_compatibility();
}
#else
namespace glm = vkm;
#endif

void VulkanCore::Init(EngineCore::IApp* pApp)
{
    if (!pApp) {
        throw std::runtime_error("no application!");
    }
    m_pApp = pApp;

#if USE_GLM
    vkm::test_vkm_glm_compatibility();
#endif
    // Some Global Initialization
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();

    createSwapChain();
    createSwapchainImageViews();

    createCommandPool();
    createColorResources();
    createDepthResources();

    createDescriptorSetLayout();
    createGraphicsPipeline();

    createDescriptorPool();
    createFrameData();

    g_emptyTexture->uploadTextureToGPU(this);

    auto pMainApp = static_cast<MainApplication*>(m_pApp);
    pMainApp->GetScene()->environment->generateCubemaps(this);
}

void VulkanCore::cleanupSwapChain()
{
    colorImage.Destroy();
    depthImage.Destroy();

    if (swapChain != VK_NULL_HANDLE) {
        for (auto swapChainImage : swapChainImages) {
            vkDestroyImageView(device, swapChainImage.imageView, nullptr);
        }
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    } else {
        for (auto swapChainImage : swapChainImages) {
            swapChainImage.Destroy();
        }
    }
}

void VulkanCore::WaitIdle()
{
    vkDeviceWaitIdle(device);
}

void VulkanCore::Shutdown()
{
    WaitIdle();
    cleanupSwapChain();

    mainDeletionStack.flush();

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    for (auto descriptorSetLayout : descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        frames[i].Destroy(device);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);
    device = VK_NULL_HANDLE;

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    if (surface) {
        vkDestroySurfaceKHR(instance, *surface, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
}

void VulkanCore::recreateSwapChain()
{
    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createSwapchainImageViews();
    createColorResources();
    createDepthResources();
}

void VulkanCore::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
#ifdef __APPLE__
        .apiVersion = VK_API_VERSION_1_2, // MoltenVK only supports 1.2
#else
        .apiVersion = VK_API_VERSION_1_3, // On Windows and Linux, we can use 1.3
#endif
    };

    auto extensions = getRequiredExtensions();
    VkInstanceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };
#if __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR; // VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo {};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanCore::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
    };
}

void VulkanCore::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanCore::createSurface()
{
    surface = m_pApp->info.window->CreateSurface(instance);
}

void VulkanCore::pickPhysicalDevice()
{
    std::vector<VkPhysicalDevice> pdevices = getAllPhysicalDevices(instance);

    if (m_pApp->args.physicalDeviceName.has_value()) {
        for (const auto& pdevice : pdevices) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(pdevice, &properties);
            if (std::string(properties.deviceName) == m_pApp->args.physicalDeviceName.value()) {
                if (!isDeviceSuitable(pdevice, surface)) {
                    std::cout << "The device given in args is not suitable: " << properties.deviceName << std::endl;
                    throw std::runtime_error("The device given in args is not suitable");
                }
                physicalDevice = pdevice;
                msaaSamples = getMaxUsableSampleCount();
                break;
            }
        }
    }

    for (const auto& pdevice : pdevices) {
        if (isDeviceSuitable(pdevice, surface)) {
            physicalDevice = pdevice;
            msaaSamples = getMaxUsableSampleCount();
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

VkSampleCountFlagBits VulkanCore::getMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) {
        return VK_SAMPLE_COUNT_64_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_32_BIT) {
        return VK_SAMPLE_COUNT_32_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_16_BIT) {
        return VK_SAMPLE_COUNT_16_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_8_BIT) {
        return VK_SAMPLE_COUNT_8_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_4_BIT) {
        return VK_SAMPLE_COUNT_4_BIT;
    }
    if (counts & VK_SAMPLE_COUNT_2_BIT) {
        return VK_SAMPLE_COUNT_2_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

void VulkanCore::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    std::set<uint32_t> uniqueQueueFamilies = indices.getUniqueIndices();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority,
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures {
        .samplerAnisotropy = VK_TRUE,
    };

    constexpr VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .dynamicRendering = VK_TRUE,
    };

    auto* enabledDeviceExtensions = IsHeadless() ? &deviceExtensionsWithoutSwapchain : &deviceExtensions;

    VkDeviceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &dynamic_rendering_feature,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(enabledDeviceExtensions->size()),
        .ppEnabledExtensionNames = enabledDeviceExtensions->data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    if (indices.presentFamily) {
        vkGetDeviceQueue(device, *indices.presentFamily, 0, &presentQueue);
    }
}

void VulkanCore::createSwapChain()
{
    if (IsHeadless()) {
        // Headless mode does not have a surface, make a fake swap chain
        int width, height;
        m_pApp->info.window->GetWindowSize(width, height);
        VkExtent2D extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        uint32_t imageCount = 3;
        swapChainImages.resize(imageCount);

        for (auto& NewSwapChainImage : swapChainImages) {
            NewSwapChainImage.Init(this, extent, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        }
    } else {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, *surface);
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        swapChainImages.resize(imageCount);

        VkSwapchainCreateInfoKHR createInfo {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = *surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .preTransform = swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
        };

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        auto queueFamilyIndices = indices.getAllIndices();

        if (queueFamilyIndices.size() == 2 && indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        std::vector<VkImage> swapChainImagesTemp(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImagesTemp.data());

        // Borrow the Image class to save SwapchainImages
        for (size_t i = 0; i < imageCount; i++) {
            // Here swapChainImages are not actually initialized by Image::Init() function, so we need to manually set the imageInfo
            swapChainImages[i].m_isValid = true;
            swapChainImages[i].m_pVulkanCore = this;
            swapChainImages[i].image = swapChainImagesTemp[i];
            swapChainImages[i].m_imageInfo = VkImageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .flags = 0,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = surfaceFormat.format,
                .extent = { extent.width, extent.height, 1 },
                .mipLevels = 1,
                .arrayLayers = createInfo.imageArrayLayers,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = createInfo.imageUsage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            };
        }
    }
}

void VulkanCore::createSwapchainImageViews()
{
    for (auto& swapChainImage : swapChainImages) {
        swapChainImage.InitImageView(swapChainImage.m_imageInfo.format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void VulkanCore::createDescriptorSetLayout()
{
    // set = 0
    {
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }, // UBO
			{ 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			{ 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			{ 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			{ 4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
			{ 5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
		};

        VkDescriptorSetLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
            .pBindings = setLayoutBindings.data(),
        };

        VK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[0]));
    }

    // set = 1
    {
        std::array<VkDescriptorSetLayoutBinding, 4> bindings;

        for (uint32_t i = 0; i < bindings.size(); ++i) {
            bindings[i] = {
                .binding = i,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr,
            };
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data(),
        };

        VK(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[1]));
    }
}

void VulkanCore::createGraphicsPipeline()
{
    // Shader vertexShader(device, "s72.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    // Shader fragmentShader(device, "s72.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    // VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShader.getShaderStageInfo(), fragmentShader.getShaderStageInfo() };

    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path shaderPath = cwd / "shader_build";
    std::filesystem::path vertShaderPath = shaderPath / "s72.vert.spv";
    std::filesystem::path fragShaderPath = shaderPath / "s72.frag.spv";

    auto vertShaderCode = readShaderFile(vertShaderPath);
    auto fragShaderCode = readShaderFile(fragShaderPath);

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // VkPipelineVertexInputStateCreateInfo vertexInputInfo = getVertexInputInfo();
    auto bindingDescription = NewVertex::getBindingDescription();
    auto attributeDescriptions = NewVertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    VkPipelineViewportStateCreateInfo viewportState {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };
    VkPipelineRasterizationStateCreateInfo rasterizer {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };
    VkPipelineMultisampleStateCreateInfo multisampling {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = msaaSamples,
        .sampleShadingEnable = VK_FALSE,
    };

    VkPipelineDepthStencilStateCreateInfo depthStencil {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {}, // Optional
        .back = {}, // Optional
        .minDepthBounds = 0.0f, // Optional
        .maxDepthBounds = 1.0f, // Optional
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo colorBlending {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    VkPushConstantRange push_constant {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(SPushConstant),
    };

    // A pipeline is defined by a pipeline layout, which specifies the complete set of resources that can be accessed by a pipeline.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 2,
        .pSetLayouts = &descriptorSetLayouts[0],
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant,
    };
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    const VkPipelineRenderingCreateInfo pipeline_rendering_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorImage.m_imageInfo.format,
        .depthAttachmentFormat = depthImage.m_imageInfo.format
    };

    VkGraphicsPipelineCreateInfo pipelineInfo {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &pipeline_rendering_create_info,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = nullptr,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
    };

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void VulkanCore::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
    };

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void VulkanCore::createColorResources()
{
    VkFormat colorFormat = swapChainImages[0].m_imageInfo.format;

    colorImage.Init(this, swapChainImages[0].GetImageExtent(), 1, msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    colorImage.InitImageView(colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    colorImage.InitImageSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

void VulkanCore::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();
    depthImage.Init(this, swapChainImages[0].GetImageExtent(), 1, msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    depthImage.InitImageView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    depthImage.InitImageSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
}

VkFormat VulkanCore::findDepthFormat()
{
    return findSupportedFormat(physicalDevice,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void VulkanCore::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes {
        VkDescriptorPoolSize {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT + MAX_MATERIAL_TYPES * MAX_DESCRIPTORS_IN_MATERIAL),
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT + MAX_MATERIAL_TYPES * MAX_DESCRIPTORS_IN_MATERIAL),
        }
    };

    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT + MAX_MATERIAL_TYPES),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

// Initialize Frame related data
void VulkanCore::createFrameData()
{
    frames = std::vector<FrameData>(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createCommandBuffer(device, commandPool, frames[i].commandBuffer);
        createDescriptorSet(device, descriptorSetLayouts[0], descriptorPool, frames[i].descriptorSet);
        createFrameSyncObjects(frames[i].imageAvailableSemaphore, frames[i].renderFinishedSemaphore, frames[i].swapchainImageFence);
        createUniformBuffers(frames[i].uniformBuffer);
    }
}

void VulkanCore::createUniformBuffers(Buffer& uniformBuffer)
{
    VkDeviceSize bufferSize = sizeof(CameraUBO);
    uniformBuffer.Init(this, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
}

void VulkanCore::createFrameSyncObjects(VkSemaphore& imageAvailableSemaphore, VkSemaphore& renderFinishedSemaphore, VkFence& swapchainImageFence)
{
    VkSemaphoreCreateInfo semaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore));
    VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore));
    VK(vkCreateFence(device, &fenceInfo, nullptr, &swapchainImageFence));
}

VkCommandBuffer VulkanCore::beginSingleTimeCommands() const
{
    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void VulkanCore::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

uint32_t VulkanCore::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties = getAllMemoryProperties(physicalDevice);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanCore::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene& scene)
{
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    if (!IsHeadless())
        swapChainImages[imageIndex].TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    const VkRenderingAttachmentInfo color_attachment_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = colorImage.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
        .resolveImageView = swapChainImages[imageIndex].imageView,
        .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {
            .color = { { 0.0f, 0.0f, 0.0f, 1.0f } },
        },
    };

    const VkRenderingAttachmentInfo depth_attachment_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = depthImage.imageView,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue = {
            .depthStencil = { 1.0f, 0 },
        },
    };

    const VkRenderingInfo render_info {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .renderArea = {
            .offset = { 0, 0 },
            .extent = { swapChainImages[0].m_imageInfo.extent.width, swapChainImages[0].m_imageInfo.extent.height } },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_info,
        .pDepthAttachment = &depth_attachment_info,
    };

    BeginRendering(commandBuffer, render_info);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width = float(swapChainImages[0].m_imageInfo.extent.width),
        .height = float(swapChainImages[0].m_imageInfo.extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor {
        .offset = { 0, 0 },
        .extent = {
            .width = swapChainImages[0].m_imageInfo.extent.width,
            .height = swapChainImages[0].m_imageInfo.extent.height },
    };

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frames[currentFrameInFlight].descriptorSet, 0, nullptr);

    // TODO: Add environment map support

    std::vector<MeshInstance> meshInstances;
    scene.Traverse(meshInstances);
#if VERBOSE
    static size_t totalMeshCount = 0;
    if (totalMeshCount != meshInstances.size()) {
        totalMeshCount = meshInstances.size();
        std::cout << "MeshInstances: " << totalMeshCount << std::endl;
    }
#endif
    if (m_pApp->args.cullingType == "frustum") {
        std::vector<MeshInstance> meshInstancesCulled;
        for (auto& MeshInst : meshInstances) {
            // Frustum Culling
            if (CameraManager::GetInstance().GetActiveCamera()->FrustumCulling(MeshInst.pMesh, MeshInst.matWorld))
                meshInstancesCulled.push_back(MeshInst);
        }
        meshInstances = std::move(meshInstancesCulled);
    }

#if VERBOSE
    auto totalMeshCountAfterCulling = meshInstances.size();
    static size_t lastMeshInstanceCount = totalMeshCountAfterCulling;
    if (lastMeshInstanceCount != totalMeshCountAfterCulling) {
        lastMeshInstanceCount = totalMeshCountAfterCulling;
        std::cout << "MeshInstances: " << totalMeshCountAfterCulling << "/" << totalMeshCount << std::endl;
    }
#endif

    for (auto& MeshInst : meshInstances) {
        auto& meshData = MeshInst.pMesh->meshData;

        // This is a lazy way to handle the case where the mesh data is not yet uploaded to the GPU
        if (!meshData->uploadModelToGPU(this))
            continue;

        // mesh - material - texture & descriptor set
        auto pMaterial = MeshInst.pMesh->GetMaterial();
        if (pMaterial == nullptr) {
            std::cerr << "Material is nullptr" << std::endl;
            continue;
        }
        pMaterial->InitDescriptorSet(this);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &pMaterial->descriptorSet, 0, nullptr);

        SPushConstant pushConstant = {
            .matWorld = MeshInst.matWorld,
            .matNormal = vkm::transpose(vkm::inverse(MeshInst.matWorld))
        };

        pushConstant.matNormal[3][3] = static_cast<float>(MeshInst.pMesh->GetMaterialType()); // A temp hack to pass material type to shader

        // upload the matrix to the GPU via push constants
        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SPushConstant), &pushConstant);

        meshData->draw(commandBuffer);
    }

    EndRendering(commandBuffer);


    if (!IsHeadless())
        swapChainImages[imageIndex].TransitionLayout(commandBuffer, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    VK(vkEndCommandBuffer(commandBuffer));
}

void VulkanCore::BeginRendering(VkCommandBuffer& commandBuffer, const VkRenderingInfo render_info)
{
#if __APPLE__
	auto vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetInstanceProcAddr(instance, "vkCmdBeginRenderingKHR");
	if (vkCmdBeginRenderingKHR == nullptr) {
		throw std::runtime_error("failed to load vkCmdBeginRenderingKHR");
	}
    vkCmdBeginRenderingKHR(commandBuffer, &render_info);
#else
	vkCmdBeginRendering(commandBuffer, &render_info);
#endif
}

void VulkanCore::EndRendering(VkCommandBuffer& commandBuffer)
{
#if __APPLE__
	auto vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetInstanceProcAddr(instance, "vkCmdEndRenderingKHR");
	if (vkCmdEndRenderingKHR == nullptr) {
		throw std::runtime_error("failed to load vkCmdEndRenderingKHR");
	}
	vkCmdEndRenderingKHR(commandBuffer);
#else
	vkCmdEndRendering(commandBuffer);
#endif
}

void VulkanCore::updateUniformBuffer(uint32_t currentImage)
{
    auto camera = CameraManager::GetInstance().GetActiveCamera();

#ifndef NDEBUG
    if (CameraManager::GetInstance().IsDebugModeActive())
        camera = CameraManager::GetInstance().GetDebugCamera();
#endif

    vkm::vec3 cameraPos = camera->getPosition();
    CameraUBO ubo {
        .view = camera->getViewMatrix(),
        .proj = camera->getProjectionMatrix(),
        .viewproj = camera->getProjectionMatrix() * camera->getViewMatrix(),
        .position = vkm::vec4(cameraPos.r(), cameraPos.g(), cameraPos.b(), 1.0f),
    };

    assert(frames[currentFrameInFlight].uniformBuffer.m_pMappedData);
    memcpy(*frames[currentFrameInFlight].uniformBuffer.m_pMappedData, &ubo, sizeof(ubo));
}

void VulkanCore::updateDescriptorSet(uint32_t currentFrameInFlight, Scene& scene)
{
    VkDescriptorBufferInfo bufferInfo {
        .buffer = frames[currentFrameInFlight].uniformBuffer.buffer,
        .offset = 0,
        .range = sizeof(CameraUBO),
    };

    scene.environment->radiance.uploadTextureToGPU(this);
	scene.environment->lambertian.uploadTextureToGPU(this);
	scene.environment->irradiance.uploadTextureToGPU(this);
	scene.environment->preFilteredEnv.uploadTextureToGPU(this);
	scene.environment->lutBrdf.uploadTextureToGPU(this);
    VkDescriptorImageInfo radianceImageInfo = scene.environment->radiance.textureImage.GetDescriptorImageInfo();
	VkDescriptorImageInfo lambertianImageInfo = scene.environment->lambertian.textureImage.GetDescriptorImageInfo();
	VkDescriptorImageInfo irradianceImageInfo = scene.environment->irradiance.textureImage.GetDescriptorImageInfo();
	VkDescriptorImageInfo prefilteredMapImageInfo = scene.environment->preFilteredEnv.textureImage.GetDescriptorImageInfo();
	VkDescriptorImageInfo lutBrdfImageInfo = scene.environment->lutBrdf.textureImage.GetDescriptorImageInfo();

    std::array<VkWriteDescriptorSet, 6> descriptorWrites {
        VkWriteDescriptorSet {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = frames[currentFrameInFlight].descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &bufferInfo,
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = frames[currentFrameInFlight].descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &radianceImageInfo,
        },
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = frames[currentFrameInFlight].descriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &lambertianImageInfo,
        },
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = frames[currentFrameInFlight].descriptorSet,
			.dstBinding = 3,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &irradianceImageInfo,
		},{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = frames[currentFrameInFlight].descriptorSet,
			.dstBinding = 4,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &prefilteredMapImageInfo,
		},{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = frames[currentFrameInFlight].descriptorSet,
			.dstBinding = 5,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &lutBrdfImageInfo,
		},
    };

    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

void VulkanCore::drawFrame(Scene& scene)
{
    bool isHeadless = IsHeadless();
    if (isHeadless) {
        if (!m_pApp->args.limitFPS)
            readyForNextImage = true;
        if (!readyForNextImage)
            return;
    }

    currentFrameInFlight = (currentFrameInFlight + 1) % MAX_FRAMES_IN_FLIGHT;
    // wait for the frame needed to use to be finished (if still in flight)
    vkWaitForFences(device, 1, &frames[currentFrameInFlight].swapchainImageFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &frames[currentFrameInFlight].swapchainImageFence);
    frames[currentFrameInFlight].deletionStack.flush(); // delete all temporary data

    uint32_t imageIndex = std::numeric_limits<uint32_t>::max(); // index of the swap chain image that will be used for the current frame

    if (isHeadless) {
        imageIndex = AcquireNextImageIndex();
    } else {
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, frames[currentFrameInFlight].imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
    }

    updateDescriptorSet(currentFrameInFlight, scene);

    updateUniformBuffer(currentFrameInFlight);
    vkResetCommandBuffer(frames[currentFrameInFlight].commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    recordCommandBuffer(frames[currentFrameInFlight].commandBuffer, imageIndex, scene);

    VkSemaphore waitSemaphores[] = { frames[currentFrameInFlight].imageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { frames[currentFrameInFlight].renderFinishedSemaphore };
    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = isHeadless ? uint32_t(0) : 1,
        .pWaitSemaphores = isHeadless ? nullptr : waitSemaphores, // will wait on these semaphores before the command buffer starts executing
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &frames[currentFrameInFlight].commandBuffer,
        .signalSemaphoreCount = isHeadless ? uint32_t(0) : 1,
        .pSignalSemaphores = isHeadless ? nullptr : signalSemaphores, // will signal these semaphores after the command buffer has finished execution
    };

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, frames[currentFrameInFlight].swapchainImageFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = { swapChain };
    VkPresentInfoKHR presentInfo {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores, // will wait on these semaphores before the image is presented
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
    };

    if (isHeadless) {
        if (m_pApp->events.windowResized) {
            m_pApp->events.windowResized = false;
            recreateSwapChain();
        }
        readyForNextImage = false;
    } else {
        VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_pApp->events.windowResized) {
            m_pApp->events.windowResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }

    return;
}

VkShaderModule VulkanCore::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

VkSurfaceFormatKHR VulkanCore::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanCore::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanCore::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;

        m_pApp->info.window->GetWindowSize(width, height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

std::vector<const char*> VulkanCore::getRequiredExtensions()
{
    std::vector<const char*> extensions;
    if (!IsHeadless()) {
#if USE_GLFW
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
#else
        // https://www.glfw.org/docs/3.3/group__vulkan.html#:~:text=glfwGetRequiredInstanceExtensions()&text=This%20function%20returns%20an%20array,Vulkan%20surfaces%20for%20GLFW%20windows.
        uint32_t glfwExtensionCount = 2;
        const char* glfwExtensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };
#endif // USE_GLFW
        extensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
    }

    // std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if __APPLE__
    // Encountered VK_ERROR_INCOMPATIBLE_DRIVER on MacOS with MoltenVK
    // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance#:~:text=is%20created%20successfully.-,Encountered%20VK_ERROR_INCOMPATIBLE_DRIVER,-%3A
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif // __APPLE__

    // VK_EXT_debug_utils
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanCore::checkValidationLayerSupport()
{
    std::vector<VkLayerProperties> availableLayers = getAllAvailableLayers();

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void VulkanCore::PresentImage()
{
    // make the least-recently-available image in the swap chain available to be rendered to (after waiting for any rendering pending on this image to complete). This, effectively, starts a frame rendering.
    nextImageIndex = (nextImageIndex + 1) % swapChainImages.size();
    readyForNextImage = true;
}

void VulkanCore::SaveFrame(const std::string& savePath)
{
    vkWaitForFences(device, 1, &frames[currentFrameInFlight].swapchainImageFence, VK_TRUE, UINT64_MAX);

    uint32_t texWidth = static_cast<uint32_t>(m_pApp->args.windowSize.first);
    uint32_t texHeight = static_cast<uint32_t>(m_pApp->args.windowSize.second);

    auto bufferData = swapChainImages[currentFrameInFlight].copyToMemory();

    // Open the file in binary mode
    std::ofstream file(savePath, std::ios::out | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + savePath);
    }

    // Write the PPM header
    file << "P6\n"
         << texWidth << " " << texHeight << "\n255\n";

    // Write the pixel data
    // PPM format expects pixels in binary RGB format, so we only need to write the RGB parts of each BGRA pixel
    for (uint32_t i = 0; i < texWidth * texHeight; ++i) {
        char* pixel = reinterpret_cast<char*>(bufferData.get() + i * 4);
        std::swap(pixel[0], pixel[2]); // Vulkan has BGRA format, PPM has RGB
        file.write(pixel, 3);
    }

    file.close();
}

uint32_t VulkanCore::AcquireNextImageIndex()
{
    return nextImageIndex;
}

void FrameData::Destroy(const VkDevice& device)
{
    // No need to destroy commandBuffer & descriptorSet because they are destroyed when the command pool and descriptor pool are destroyed
    // vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    // vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);

    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroyFence(device, swapchainImageFence, nullptr);
    uniformBuffer.Destroy();
}
