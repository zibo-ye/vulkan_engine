#include "Texture.hpp"

Texture::Texture(const Utility::json::JsonValue& jsonObj, const std::string& scenePath)
{
    std::filesystem::path scenePathFS = scenePath;
    src = (scenePathFS.parent_path() / jsonObj["src"].getString()).string();
    type = jsonObj.hasKey("type") ? jsonObj["type"].getString() : "2D"; // Default to "2D"
    format = jsonObj.hasKey("format") ? jsonObj["format"].getString() : "linear"; // Default to "linear"
    LoadTextureData();
}

Texture::Texture(const vkm::vec3& vec3Value)
{
    src = "";
    type = "2D";
    format = "linear";
    texWidth = 1;
    texHeight = 1;
    texChannels = 4; // vec3 not supported by Vulkan, so we use vec4
    mipLevels = 1;
    textureData = std::shared_ptr<stbi_uc>(new stbi_uc[4] { (stbi_uc)(vec3Value.x() * 255), (stbi_uc)(vec3Value.y() * 255), (stbi_uc)(vec3Value.z() * 255), 0 }, stbi_image_free);
}

Texture::Texture(const float& floatValue)
{
    src = "";
    type = "2D";
    format = "linear";
    texWidth = 1;
    texHeight = 1;
    texChannels = 1;
    mipLevels = 1;
    textureData = std::shared_ptr<stbi_uc>(new stbi_uc[1] { (stbi_uc)(floatValue * 255) }, stbi_image_free);
}

void Texture::LoadTextureData()
{
    stbi_uc* pixels = stbi_load(src.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha); // TODO: single channel image
    if (type == "cube") { // if it's a cube map, the width and height should be the same
        texHeight = texHeight / 6;
        assert(texWidth == texHeight);
    }

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    if (texChannels == 3) // R8G8B8 not supported, so we need to convert it to R8G8B8A8
    {
        texChannels = 4;
    }
    textureData = std::shared_ptr<stbi_uc>(pixels, stbi_image_free);
}

bool Texture::uploadTextureToGPU(VulkanCore* vulkanCore)
{
    if (isOnGPU)
        return true;
    m_pVulkanCore = vulkanCore;
    createImage();
    m_pVulkanCore->mainDeletionStack.push([=]() {
        releaseTextureFromGPU();
    });

    isOnGPU = true;
    return true;
}

bool Texture::releaseTextureFromGPU()
{
    if (!isOnGPU)
        return false;
    assert(m_pVulkanCore != nullptr);

    auto device = m_pVulkanCore->GetDevice();
    if (device != VK_NULL_HANDLE) {
        textureImage.Destroy();
    }
    isOnGPU = false;
    return true;
}

// This currently only supports RGBA8 format, can be extended to support other formats
void Texture::createImage()
{
    if (type == "2D") {
        VkDeviceSize imageSize = texWidth * texHeight * texChannels;
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // TODO: normal
        // TODO: can we infer the format from image itself?
        switch (texChannels) {
        case 1:
            format = VK_FORMAT_R8_UNORM;
            break;
        case 4:
            format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        default:
            throw std::runtime_error("Unsupported texture format");
        }

        textureImage.Init(m_pVulkanCore, VkExtent2D { (uint32_t)texWidth, (uint32_t)texHeight }, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        textureImage.InitImageView(format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        textureImage.UploadData(textureData.get(), imageSize);
    } else if (type == "cube") { // Cube map
        VkDeviceSize imageSize = texWidth * texHeight * texChannels * 6;
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // TODO: channel = 1 & normal & roughness
        textureImage.Init(m_pVulkanCore, VkExtent2D { (uint32_t)texWidth, (uint32_t)texHeight }, mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);
        textureImage.InitImageView(format, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_CUBE);
        textureImage.UploadData(textureData.get(), imageSize);
    } else {
        throw std::runtime_error("Unsupported texture type");
    }
}

std::shared_ptr<Texture> g_emptyTexture = std::make_shared<Texture>(vkm::vec3(0.9412f, 0.f, 0.3373f)); // error texture