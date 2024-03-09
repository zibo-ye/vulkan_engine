#pragma once
#include "Graphics/Vulkan/Image.hpp"
#include "Graphics/Vulkan/VulkanCore.hpp"
#include "ThirdParty/stb_image.h"
#include "pch.hpp"

class Texture {
    // Metadata from JSON
public:
    std::string src;
    std::string type;
    std::string format;
    Texture() = default;
    Texture(const Utility::json::JsonValue& jsonObj, const std::string& scenePath, VkFormat imageFormat);
    Texture(const vkm::vec3& vec3Value, VkFormat imageFormat);
    Texture(const float& floatValue, VkFormat imageFormat);

    // Texture data from src
public:
    void LoadTextureData();

    int texWidth, texHeight, texChannels;
    int mipLevels;
    std::shared_ptr<stbi_uc> textureData;

    // Vulkan
public:
    bool uploadTextureToGPU(VulkanCore* vulkanCore);
    bool releaseTextureFromGPU();

    Image textureImage;
    VkFormat textureImageFormat;

private:
    void createImage();

private:
    bool isOnGPU = false;
    VulkanCore* m_pVulkanCore = nullptr;
};

extern std::shared_ptr<Texture> g_emptyTexture;
