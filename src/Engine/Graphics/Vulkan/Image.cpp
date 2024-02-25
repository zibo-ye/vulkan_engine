#include "Image.hpp"
#include "Buffer.hpp"
#include "VulkanCore.hpp"

bool Image::Init(VulkanCore* pVulkanCore, ExtentVariant extent, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageLayout initialLayout /* = VK_IMAGE_LAYOUT_UNDEFINED */)
{
    if (!pVulkanCore)
        return false;
    m_pVulkanCore = pVulkanCore;
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .format = format,
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = numSamples,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = initialLayout,
    };

    // Determine the type of extent and set imageInfo fields accordingly
    std::visit([&imageInfo](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, uint32_t>) {
            imageInfo.imageType = VK_IMAGE_TYPE_1D;
            imageInfo.extent = { arg, 1, 1 };
        } else if constexpr (std::is_same_v<T, VkExtent2D>) {
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent = { arg.width, arg.height, 1 };
        } else if constexpr (std::is_same_v<T, VkExtent3D>) {
            imageInfo.imageType = VK_IMAGE_TYPE_3D;
            imageInfo.extent = arg;
        }
    },
        extent);

    VK(vkCreateImage(m_pVulkanCore->GetDevice(), &imageInfo, nullptr, &image));
    m_imageInfo = std::move(imageInfo);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_pVulkanCore->GetDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = m_pVulkanCore->findMemoryType(memRequirements.memoryTypeBits, properties)
    };

    VK(vkAllocateMemory(m_pVulkanCore->GetDevice(), &allocInfo, nullptr, &imageMemory));
    VK(vkBindImageMemory(m_pVulkanCore->GetDevice(), image, imageMemory, 0));

    m_isValid = true;
    m_currentLayout = initialLayout;

    // #MAYBE: m_pVulkanCore->AddResizeHandler(), m_pVulkanCore->AddShutdownHandler()
    return true;
}

void Image::Destroy()
{
    assert(m_isValid);
    vkDestroyImageView(m_pVulkanCore->GetDevice(), imageView, nullptr);
    vkDestroyImage(m_pVulkanCore->GetDevice(), image, nullptr);
    vkFreeMemory(m_pVulkanCore->GetDevice(), imageMemory, nullptr);
    m_isValid = false;
    m_currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void Image::InitImageView(VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    assert(m_isValid);
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VK(vkCreateImageView(m_pVulkanCore->GetDevice(), &viewInfo, nullptr, &imageView));
}

std::pair<VkAccessFlags, VkPipelineStageFlags> getMinimalAccessMaskAndStage(VkImageLayout layout)
{
    switch (layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        return { 0, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        return { VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        return { VK_ACCESS_COLOR_ATTACHMENT_READ_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        return { VK_ACCESS_TRANSFER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT };

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        return { VK_ACCESS_MEMORY_READ_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        return { VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT };

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        return { VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT };

    default:
        throw std::invalid_argument("Unsupported layout for access mask and stage retrieval!");
    }
}

void Image::TransitionLayout(std::optional<VkCommandBuffer> commandBuffer, VkImageLayout newLayout, uint32_t mipLevels)
{
    assert(m_isValid);
    VkCommandBuffer cmd;
    if (!commandBuffer)
        cmd = m_pVulkanCore->beginSingleTimeCommands();
    else
        cmd = *commandBuffer;

    if (newLayout == m_currentLayout)
        return;

    VkImageLayout oldLayout = m_currentLayout;
    auto [srcAccessMask, sourceStage] = getMinimalAccessMaskAndStage(oldLayout);
    auto [dstAccessMask, destinationStage] = getMinimalAccessMaskAndStage(newLayout);

    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = srcAccessMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = m_currentLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(m_imageInfo.format)) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCmdPipelineBarrier(
        cmd,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    m_currentLayout = newLayout;
    if (!commandBuffer)
        m_pVulkanCore->endSingleTimeCommands(cmd);
}

ExtentVariant Image::GetImageExtent()
{
    assert(m_isValid);
    switch (m_imageInfo.imageType) {
    case VK_IMAGE_TYPE_1D:
        // Assuming you have stored the original extent as a VkExtent3D
        return m_imageInfo.extent.width;
    case VK_IMAGE_TYPE_2D:
        return VkExtent2D { m_imageInfo.extent.width, m_imageInfo.extent.height };
    case VK_IMAGE_TYPE_3D:
        return m_imageInfo.extent; // Directly return the VkExtent3D
    default:
        throw std::runtime_error("Unsupported image type");
    }
}

void Image::CopyToBuffer(Buffer& buffer)
{
    assert(m_isValid);
    VkCommandBuffer commandBuffer = m_pVulkanCore->beginSingleTimeCommands();

    VkBufferImageCopy region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,

        .imageSubresource {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },

        .imageOffset = { 0, 0, 0 },
        .imageExtent = m_imageInfo.extent
    };

    vkCmdCopyImageToBuffer(
        commandBuffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        buffer.buffer,
        1,
        &region);

    m_pVulkanCore->endSingleTimeCommands(commandBuffer);
}

std::unique_ptr<uint8_t[]> Image::copyToMemory()
{
    VkDeviceSize imageSize = m_imageInfo.extent.width * m_imageInfo.extent.height * 4; // Assuming 4 bytes per pixel (RGBA), #TODO: can be inferred from the format

    auto bufferData = std::make_unique<uint8_t[]>(imageSize);

    // Create a helper staging buffer to copy the image data to the CPU
    Buffer stagingBuffer;
    stagingBuffer.Init(m_pVulkanCore, imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    auto oldLayout = m_currentLayout;
    this->TransitionLayout(std::nullopt, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 1);
    this->CopyToBuffer(stagingBuffer);
    this->TransitionLayout(std::nullopt, oldLayout, 1);

    void* data = stagingBuffer.Map();
    memcpy(bufferData.get(), data, static_cast<size_t>(imageSize)); // Copy data into the unique_ptr buffer
    stagingBuffer.Unmap();
    stagingBuffer.Destroy();

    return bufferData; // Return the unique_ptr containing the copied texture data
}

// it is uncommon in practice to generate the mipmap levels at runtime, but here it is.
void Image::GenerateMipmaps(std::optional<VkCommandBuffer> commandBuffer, uint32_t mipLevels)
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_pVulkanCore->GetPhysicalDevice(), m_imageInfo.format, &formatProperties);
    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    if (!commandBuffer)
        commandBuffer = m_pVulkanCore->beginSingleTimeCommands();

    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    int32_t mipWidth = m_imageInfo.extent.width;
    int32_t mipHeight = m_imageInfo.extent.height;
    int32_t mipDepth = m_imageInfo.extent.depth;

    // generate mipmap using blit from 1->2, 2->3, ...
    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(*commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
        VkImageBlit blit {
            .srcSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i - 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .srcOffsets = { VkOffset3D { 0, 0, 0 }, { mipWidth, mipHeight, mipDepth } },
            .dstSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = i,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstOffsets = { VkOffset3D { 0, 0, 0 }, { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, mipDepth > 1 ? mipDepth / 2 : 1 } },
        };
        vkCmdBlitImage(*commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,

        vkCmdPipelineBarrier(*commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
        if (mipDepth > 1)
            mipDepth /= 2;
    }

    // handle last level mipmap barrier
    barrier.subresourceRange.baseMipLevel = mipLevels - 1,
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,

    vkCmdPipelineBarrier(*commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    m_pVulkanCore->endSingleTimeCommands(*commandBuffer);
}
