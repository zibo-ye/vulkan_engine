#include "Texture.hpp"

Texture::Texture(const Utility::json::JsonValue& jsonObj, const std::string& scenePath, VkFormat imageFormat)
{
    std::filesystem::path scenePathFS = scenePath;
    src = (scenePathFS.parent_path() / jsonObj["src"].getString()).string();
    type = jsonObj.hasKey("type") ? jsonObj["type"].getString() : "2D"; // Default to "2D", can be "cube"
    format = jsonObj.hasKey("format") ? jsonObj["format"].getString() : "linear"; // Default to "linear", can be "rgbe"
    textureImageFormat = imageFormat;
    LoadTextureData();
}

Texture::Texture(const vkm::vec3& vec3Value, VkFormat imageFormat)
{
    src = "";
    type = "2D";
    format = "linear";
    textureImageFormat = imageFormat;
    texWidth = 1;
    texHeight = 1;
    texChannels = 4; // vec3 not supported by Vulkan, so we use vec4
    mipLevels = 1;
    textureData = std::shared_ptr<stbi_uc>(new stbi_uc[4] { (stbi_uc)(vec3Value.x() * 255), (stbi_uc)(vec3Value.y() * 255), (stbi_uc)(vec3Value.z() * 255), 0 }, stbi_image_free);
}

Texture::Texture(const float& floatValue, VkFormat imageFormat)
{
    src = "";
    type = "2D";
    format = "linear";
    textureImageFormat = imageFormat;
    texWidth = 1;
    texHeight = 1;
    texChannels = 1;
    mipLevels = 1;
    textureData = std::shared_ptr<stbi_uc>(new stbi_uc[1] { (stbi_uc)(floatValue * 255) }, stbi_image_free);
}

void Texture::LoadTextureData()
{
    stbi_uc* pixels = stbi_load(src.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha); // TODO: single channel image
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    if (texChannels == 3) // R8G8B8 not supported, so we need to convert it to R8G8B8A8
    {
        texChannels = 4;
    }

    // stbi_load returns the image in top-to-bottom order, but Vulkan expects it bottom-to-top order
    stbi_uc* rawData = new stbi_uc[4 * texWidth * texHeight];
    textureData = std::shared_ptr<stbi_uc>(rawData, [](stbi_uc* p) { delete[] p; });
    for (int y = 0; y < texHeight; y++) {
        for (int x = 0; x < texWidth; x++) {
            for (int c = 0; c < texChannels; c++) {
                textureData.get()[4 * (y * texWidth + x) + c] = pixels[4 * ((texHeight - 1 - y) * texWidth + x) + c];
            }
        }
    }

    if (type == "cube") { // if it's a cube map, the width and height should be the same
        texHeight = texHeight / 6;
        assert(texWidth == texHeight);
    }

    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
    stbi_image_free(pixels);
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

// Bad encapsulation
void Texture::createImage()
{
    if (type == "2D") {
        VkDeviceSize imageSize = texWidth * texHeight * texChannels;
        textureImage.Init(m_pVulkanCore, VkExtent2D { (uint32_t)texWidth, (uint32_t)texHeight }, mipLevels, VK_SAMPLE_COUNT_1_BIT, textureImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        textureImage.InitImageView(textureImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
        textureImage.InitImageSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        textureImage.UploadData(textureData.get(), imageSize);
        textureImage.GenerateMipmaps(std::nullopt, mipLevels);
    } else if (type == "cube") { // Cube map
        VkDeviceSize imageSize = texWidth * texHeight * texChannels * 6;
        textureImage.Init(m_pVulkanCore, VkExtent2D { (uint32_t)texWidth, (uint32_t)texHeight }, mipLevels, VK_SAMPLE_COUNT_1_BIT, textureImageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);
        textureImage.InitImageView(textureImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, VK_IMAGE_VIEW_TYPE_CUBE);
        textureImage.InitImageSampler(VK_SAMPLER_ADDRESS_MODE_REPEAT);
        textureImage.UploadData(textureData.get(), imageSize);
		textureImage.GenerateMipmaps(std::nullopt, mipLevels);
	}
	else {
        throw std::runtime_error("Unsupported texture type");
    }
}

std::shared_ptr<Texture> g_emptyTexture = std::make_shared<Texture>(vkm::vec3(0.9412f, 0.f, 0.3373f), VK_FORMAT_R8G8B8A8_SRGB); // error texture