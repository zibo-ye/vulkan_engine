#include "Environment.hpp"
#include "Graphics/Vulkan/Shader.hpp"
#include "Scene/Mesh.hpp"
#include "Utilities/lambertian/blur_cube.h"
#include <random>

Environment::Environment(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::ENVIRONMENT)
{
    name = jsonObj["name"].getString();

    radiance = Texture(jsonObj["radiance"], pScene.lock()->src, VK_FORMAT_R8G8B8A8_SRGB);

    lambertian = GenerateLambertian2(radiance);
	// lambertian = GenerateLambertian(radiance); // currently not producing the same result as GenerateLambertian2

    GetBRDFLut();
}

void Environment::generateCubemaps(VulkanCore* pVulkanCore)
{
    enum Target { IRRADIANCE = 0,
        PREFILTEREDENV = 1 };

    auto* device = pVulkanCore->GetDevice();

    for (uint32_t target = 0; target < PREFILTEREDENV + 1; target++) {
        auto tStart = std::chrono::high_resolution_clock::now();

        VkFormat format;
        int32_t dim;

        switch (target) {
        case IRRADIANCE:
			format = VK_FORMAT_R8G8B8A8_SRGB;
			//format = VK_FORMAT_R32G32B32A32_SFLOAT;
            dim = 64;
            break;
        case PREFILTEREDENV:
			format = VK_FORMAT_R8G8B8A8_SRGB;
            dim = 512;
            break;
        };

        const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

        Texture cubeMap = Texture();
        cubeMap.src = "";
        cubeMap.type = "cube";
        cubeMap.format = "rgbe";
        cubeMap.textureImageFormat = format;
        // cubeMap.LoadTextureData();

        cubeMap.texWidth = dim;
        cubeMap.texHeight = dim;
        cubeMap.texChannels = 4;
        cubeMap.mipLevels = numMips;

        stbi_uc* rawData = new stbi_uc[4 * dim * dim * 6];
        cubeMap.textureData = std::shared_ptr<stbi_uc>(rawData, [](stbi_uc* p) { delete[] p; });

        cubeMap.uploadTextureToGPU(pVulkanCore);

		// FB, Att, RP, Pipe, etc.
		VkAttachmentDescription attDesc{};
		// Color attachment
		attDesc.format = format;
		attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Renderpass
		VkRenderPassCreateInfo renderPassCI{};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attDesc;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;
		renderPassCI.dependencyCount = 2;
		renderPassCI.pDependencies = dependencies.data();
		VkRenderPass renderpass;
		VK(vkCreateRenderPass(device, &renderPassCI, nullptr, &renderpass));


        Image offscreenImage = Image();
        offscreenImage.Init(pVulkanCore, VkExtent2D { (uint32_t)dim, (uint32_t)dim }, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        offscreenImage.InitImageView(format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

        offscreenImage.TransitionLayout(std::nullopt, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = renderpass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &offscreenImage.imageView;
		framebufferCI.width = dim;
		framebufferCI.height = dim;
		framebufferCI.layers = 1;
		VkFramebuffer framebuffer;
        VK(vkCreateFramebuffer(device, &framebufferCI, nullptr, &framebuffer));

        // Descriptors
        VkDescriptorSetLayout descriptorsetlayout;
        VkDescriptorSetLayoutBinding setLayoutBinding = { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr };
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &setLayoutBinding,
        };
        VK(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorsetlayout));

        // Descriptor Pool
        VkDescriptorPoolSize poolSize = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 };
        VkDescriptorPoolCreateInfo descriptorPoolCI {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 2,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize,
        };

        VkDescriptorPool descriptorpool;
        VK(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &descriptorpool));

        // Descriptor sets
        VkDescriptorSet descriptorset;
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorpool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorsetlayout,
        };

        VK(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorset));
		radiance.uploadTextureToGPU(pVulkanCore);
        VkDescriptorImageInfo imageInfo = radiance.textureImage.GetDescriptorImageInfo(); // TODO: textures.environmentCube.descriptor
        VkWriteDescriptorSet descriptorWrite {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorset,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);

        struct PushBlockIrradiance {
            vkm::mat4 mvp;
            float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
            float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
        } pushBlockIrradiance;

        struct PushBlockPrefilterEnv {
            vkm::mat4 mvp;
            float roughness;
            uint32_t numSamples = 32u;
        } pushBlockPrefilterEnv;

        // Pipeline layout
        VkPipelineLayout pipelinelayout;
        VkPushConstantRange pushConstantRange {};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        switch (target) {
        case IRRADIANCE:
            pushConstantRange.size = sizeof(PushBlockIrradiance);
            break;
        case PREFILTEREDENV:
            pushConstantRange.size = sizeof(PushBlockPrefilterEnv);
            break;
        };

        VkPipelineLayoutCreateInfo pipelineLayoutCI {};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
        pipelineLayoutCI.pushConstantRangeCount = 1;
        pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
        VK(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelinelayout));

        // Pipeline
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI {};
        inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCI {};
        rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
        rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateCI.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blendAttachmentState {};
        blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachmentState.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCI {};
        colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCI.attachmentCount = 1;
        colorBlendStateCI.pAttachments = &blendAttachmentState;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCI {};
        depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCI.depthTestEnable = VK_FALSE;
        depthStencilStateCI.depthWriteEnable = VK_FALSE;
        depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilStateCI.front = depthStencilStateCI.back;
        depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

        VkPipelineViewportStateCreateInfo viewportStateCI {};
        viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCI.viewportCount = 1;
        viewportStateCI.scissorCount = 1;

        VkPipelineMultisampleStateCreateInfo multisampleStateCI {};
        multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicStateCI {};
        dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
        dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

        // Vertex input state
        auto bindingDescription = NewVertex::getBindingDescription();
        auto attributeDescriptions = NewVertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
        };

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
        Shader vertexShader(device, "filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        Shader fragmentShader(device, target == IRRADIANCE ? "irradiancecube.frag.spv" : "prefilterenvmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        shaderStages[0] = vertexShader.getShaderStageInfo();
        shaderStages[1] = fragmentShader.getShaderStageInfo();

        VkGraphicsPipelineCreateInfo pipelineCI {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputStateCI,
            .pInputAssemblyState = &inputAssemblyStateCI,
            .pViewportState = &viewportStateCI,
            .pRasterizationState = &rasterizationStateCI,
            .pMultisampleState = &multisampleStateCI,
            .pDepthStencilState = &depthStencilStateCI,
            .pColorBlendState = &colorBlendStateCI,
            .pDynamicState = &dynamicStateCI,
            .layout = pipelinelayout,
            .renderPass = renderpass,
        };

        VkPipeline pipeline;
        VK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline));

        // Render cubemap
		VkClearValue clearValues[1];
		clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderpass;
		renderPassBeginInfo.framebuffer = framebuffer;
		renderPassBeginInfo.renderArea.extent.width = dim;
		renderPassBeginInfo.renderArea.extent.height = dim;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;

        std::vector<vkm::mat4> matrices = {
            vkm::rotate(vkm::rotate(vkm::mat4(), vkm::radians(90.0f), vkm::vec3(0.0f, 1.0f, 0.0f)), vkm::radians(180.0f), vkm::vec3(1.0f, 0.0f, 0.0f)),
            vkm::rotate(vkm::rotate(vkm::mat4(), vkm::radians(-90.0f), vkm::vec3(0.0f, 1.0f, 0.0f)), vkm::radians(180.0f), vkm::vec3(1.0f, 0.0f, 0.0f)),
            vkm::rotate(vkm::mat4(), vkm::radians(-90.0f), vkm::vec3(1.0f, 0.0f, 0.0f)),
            vkm::rotate(vkm::mat4(), vkm::radians(90.0f), vkm::vec3(1.0f, 0.0f, 0.0f)),
            vkm::rotate(vkm::mat4(), vkm::radians(180.0f), vkm::vec3(1.0f, 0.0f, 0.0f)),
            vkm::rotate(vkm::mat4(), vkm::radians(180.0f), vkm::vec3(0.0f, 0.0f, 1.0f)),
        };

        VkViewport viewport {};
        viewport.width = (float)dim;
        viewport.height = (float)dim;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor {};
        scissor.extent.width = dim;
        scissor.extent.height = dim;

		cubeMap.textureImage.TransitionLayout(std::nullopt, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		for (uint32_t m = 0; m < numMips; m++) {
            printf("Generating mip %d\n", m);
            for (uint32_t f = 0; f < 6; f++) {
                printf("Generating face %d\n", f);

                VkCommandBuffer cmdBuf = pVulkanCore->beginSingleTimeCommands();

				vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


                viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
                viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
                vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
                vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

                // Pass parameters for current pass using a push constant block
                switch (target) {
                case IRRADIANCE:
                    pushBlockIrradiance.mvp = vkm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
                    vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockIrradiance), &pushBlockIrradiance);
                    break;
                case PREFILTEREDENV:
                    pushBlockPrefilterEnv.mvp = vkm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
                    pushBlockPrefilterEnv.roughness = (float)m / (float)(numMips - 1);
                    vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockPrefilterEnv), &pushBlockPrefilterEnv);
                    break;
                };

                vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorset, 0, NULL);

                VkDeviceSize offsets[1] = { 0 };

                auto box = Scene::defaultScene()->meshes.begin()->second->meshData;
                box->uploadModelToGPU(pVulkanCore);
                box->draw(cmdBuf);
				vkCmdEndRenderPass(cmdBuf);

                // Copy region for transfer from framebuffer to cube face
                VkImageCopy copyRegion {};

                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };

                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = f;
                copyRegion.dstSubresource.mipLevel = m;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };

                copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
                copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
                copyRegion.extent.depth = 1;

                offscreenImage.CopyToImage(cubeMap.textureImage, copyRegion);

            }
        }

        cubeMap.textureImage.TransitionLayout(std::nullopt, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyRenderPass(device, renderpass, nullptr);
		vkDestroyFramebuffer(device, framebuffer, nullptr);
        offscreenImage.Destroy();
        vkDestroyDescriptorPool(device, descriptorpool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorsetlayout, nullptr);
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelinelayout, nullptr);

        switch (target) {
        case IRRADIANCE:
            irradiance = std::move(cubeMap);
            break;
        case PREFILTEREDENV:
            preFilteredEnv = std::move(cubeMap);
            // shaderValuesParams.prefilteredCubeMipLevels = static_cast<float>(numMips);
            break;
        };

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        std::cout << "Generating cube map with " << numMips << " mip levels took " << tDiff << " ms" << std::endl;
    }
}

void Environment::GetBRDFLut()
{
	auto in_file = radiance.src;
	// outfilename = in_file's folder + brdflut.png
	std::filesystem::path p(in_file);
	std::string out_file = p.parent_path().string() + "/brdflut.png";

	lutBrdf = Texture();
	lutBrdf.src = out_file;
	lutBrdf.type = "2D";
	lutBrdf.format = "linear";
	lutBrdf.textureImageFormat = VK_FORMAT_R8G8B8A8_SRGB;
	lutBrdf.LoadTextureData();
}

enum class CubeMapFace {
    PositiveX = 0,
    NegativeX = 1,
    PositiveY = 2,
    NegativeY = 3,
    PositiveZ = 4,
    NegativeZ = 5
};

inline vkm::vec3 rgbe_to_float(vkm::u8vec4 col)
{
    // avoid decoding zero to a denormalized value:
    if (col == vkm::u8vec4(0, 0, 0, 0))
        return vkm::vec3(0.0f);

    int exp = int(col.a()) - 128;
    return vkm::vec3(
        std::ldexp((col.r() + 0.5f) / 256.0f, exp),
        std::ldexp((col.g() + 0.5f) / 256.0f, exp),
        std::ldexp((col.b() + 0.5f) / 256.0f, exp));
}

inline vkm::u8vec4 float_to_rgbe(vkm::vec3 col)
{

    float d = std::max(col.r(), std::max(col.g(), col.b()));

    // 1e-32 is from the radiance code, and is probably larger than strictly necessary:
    if (d <= 1e-32f) {
        return vkm::u8vec4(0, 0, 0, 0);
    }

    int e;
    float fac = 255.999f * (std::frexp(d, &e) / d);

    // value is too large to represent, clamp to bright white:
    if (e > 127) {
        return vkm::u8vec4(0xff, 0xff, 0xff, 0xff);
    }

    // scale and store:
    return vkm::u8vec4(
        std::max(0, int32_t(col.r() * fac)),
        std::max(0, int32_t(col.g() * fac)),
        std::max(0, int32_t(col.b() * fac)),
        e + 128);
}

// Adopted from blur_cube.cpp: https://github.com/ixchow/15-466-ibl/blob/master/cubes/blur_cube.cpp
Texture GenerateLambertian(Texture radiance, int lambertian_texWidth /*= 16*/)
{
    Texture lambertian = Texture();
    lambertian.type = radiance.type;
    lambertian.format = radiance.format;
    lambertian.textureImageFormat = radiance.textureImageFormat;

    lambertian.texWidth = lambertian_texWidth;
    lambertian.texHeight = lambertian_texWidth;
    lambertian.texChannels = radiance.texChannels;
    lambertian.mipLevels = 1;

    stbi_uc* rawData = new stbi_uc[4 * lambertian_texWidth * lambertian_texWidth * 6];
    lambertian.textureData = std::shared_ptr<stbi_uc>(rawData, [](stbi_uc* p) { delete[] p; });

    // Define sampling and bright direction handling as in the provided snippet
    std::function<vkm::vec3()> make_sample;

    // Initialize sampling based on diffuse mode (as an example)
    make_sample = []() -> vkm::vec3 {
        static std::mt19937 mt(0x12341234);
        vkm::vec2 rv(mt() / float(mt.max()), mt() / float(mt.max()));
        float phi = rv.x() * 2.0f * M_PI;
        float r = std::sqrt(rv.y());
        return vkm::vec3(
            std::cos(phi) * r,
            std::sin(phi) * r,
            std::sqrt(1.0f - rv.y()));
    };

    // Convert radiance to linear data
    std::vector<vkm::vec3> linearData(radiance.texWidth * radiance.texHeight * 6);

    for (int i = 0; i < radiance.texWidth * radiance.texHeight * 6; ++i) {
        vkm::u8vec4 rgbe = vkm::u8vec4(
            radiance.textureData.get()[4 * i + 0],
            radiance.textureData.get()[4 * i + 1],
            radiance.textureData.get()[4 * i + 2],
            radiance.textureData.get()[4 * i + 3]);
        linearData[i] = rgbe_to_float(rgbe);
    }

    int in_size = radiance.texWidth;

    // function for sampling a given direction from cubemap:
    auto lookup = [&linearData, &in_size](vkm::vec3 const& dir) -> vkm::vec3 {
        float sc, tc, ma;
        uint32_t f;
        if (std::abs(dir.x()) >= std::abs(dir.y()) && std::abs(dir.x()) >= std::abs(dir.z())) {
            if (dir.x() >= 0) {
                sc = -dir.z();
                tc = -dir.y();
                ma = dir.x();
                f = static_cast<uint32_t>(CubeMapFace::PositiveX);
            } else {
                sc = dir.z();
                tc = -dir.y();
                ma = -dir.x();
                f = static_cast<uint32_t>(CubeMapFace::NegativeX);
            }
        } else if (std::abs(dir.y()) >= std::abs(dir.z())) {
            if (dir.y() >= 0) {
                sc = dir.x();
                tc = dir.z();
                ma = dir.y();
                f = static_cast<uint32_t>(CubeMapFace::PositiveY);
            } else {
                sc = dir.x();
                tc = -dir.z();
                ma = -dir.y();
                f = static_cast<uint32_t>(CubeMapFace::NegativeY);
            }
        } else {
            if (dir.z() >= 0) {
                sc = dir.x();
                tc = -dir.y();
                ma = dir.z();
                f = static_cast<uint32_t>(CubeMapFace::PositiveZ);
            } else {
                sc = -dir.x();
                tc = -dir.y();
                ma = -dir.z();
                f = static_cast<uint32_t>(CubeMapFace::NegativeZ);
            }
        }

        int32_t s = static_cast<int32_t>(std::floor(0.5f * (sc / ma + 1.0f) * in_size));
        s = std::max(0, std::min(int32_t(in_size) - 1, s));
        int32_t t = static_cast<int32_t>(std::floor(0.5f * (tc / ma + 1.0f) * in_size));
        t = std::max(0, std::min(int32_t(in_size) - 1, t));

        return linearData[(f * in_size + t) * in_size + s];
    };

    for (int i = 0; i < 6; i++) {
        // std::cout << "Sampling face " << i << "/6 ..." << std::endl;
        vkm::vec3 sc; // maps to rightward axis on face
        vkm::vec3 tc; // maps to upward axis on face
        vkm::vec3 ma; // direction to face
        CubeMapFace face = static_cast<CubeMapFace>(i);
        if (face == CubeMapFace::PositiveX) {
            sc = vkm::vec3(0.0f, 0.0f, -1.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(1.0f, 0.0f, 0.0f);
        } else if (face == CubeMapFace::NegativeX) {
            sc = vkm::vec3(0.0f, 0.0f, 1.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(-1.0f, 0.0f, 0.0f);
        } else if (face == CubeMapFace::PositiveY) {
            sc = vkm::vec3(1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, 0.0f, 1.0f);
            ma = vkm::vec3(0.0f, 1.0f, 0.0f);
        } else if (face == CubeMapFace::NegativeY) {
            sc = vkm::vec3(1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, 0.0f, -1.0f);
            ma = vkm::vec3(0.0f, -1.0f, 0.0f);
        } else if (face == CubeMapFace::PositiveZ) {
            sc = vkm::vec3(1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(0.0f, 0.0f, 1.0f);
        } else if (face == CubeMapFace::NegativeZ) {
            sc = vkm::vec3(-1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(0.0f, 0.0f, -1.0f);
        } else
            assert(0 && "Invalid face.");

        for (int y = 0; y < lambertian_texWidth; y++) {
            for (int x = 0; x < lambertian_texWidth; x++) {

                vkm::vec3 N = (ma
                    + (2.0f * (x + 0.5f) / lambertian_texWidth - 1.0f) * sc
                    + (2.0f * (y + 0.5f) / lambertian_texWidth - 1.0f) * tc);
                N = vkm::normalize(N);
                vkm::vec3 temp = (abs(N.z()) < 0.99f ? vkm::vec3(0.0f, 0.0f, 1.0f) : vkm::vec3(1.0f, 0.0f, 0.0f));
                vkm::vec3 TX = vkm::normalize(vkm::cross(N, temp));
                vkm::vec3 TY = vkm::cross(N, TX);

                vkm::vec3 acc = vkm::vec3(0.0f);
                int samples = 1000;
                for (uint32_t i = 0; i < uint32_t(samples); ++i) {
                    // very inspired by the SampleGGX code in "Real Shading in Unreal" (https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf):

                    vkm::vec3 dir = make_sample();

                    acc += lookup(dir.x() * TX + dir.y() * TY + dir.z() * N);
                    // acc += (dir.x * TX + dir.y * TY + dir.z * N) * 0.5f + 0.5f; //DEBUG
                }

                acc *= 1.0f / float(samples);

                // convert to RGBE:
                vkm::u8vec4 rgbe = float_to_rgbe(acc);

                // store in the texture:
                int index = 4 * (x + y * lambertian_texWidth + i * lambertian_texWidth * lambertian_texWidth);
                lambertian.textureData.get()[index + 0] = rgbe.r();
                lambertian.textureData.get()[index + 1] = rgbe.g();
                lambertian.textureData.get()[index + 2] = rgbe.b();
                lambertian.textureData.get()[index + 3] = rgbe.a();
            }
        }
    }
    return lambertian;
}

Texture GenerateLambertian2(Texture radiance)
{
    glm::ivec2 out_size = glm::ivec2(16, 16 * 6);
    int32_t samples = 1024;
    int32_t brightest = 10000;
    std::string in_file = radiance.src;

    // outfilename = in_file's filename + "_lambertian" + in_file's extension
    std::string out_file = in_file.substr(0, in_file.find_last_of('.')) + "_lambertian" + in_file.substr(in_file.find_last_of('.'));

    blur_cube("diffuse", out_size, samples, in_file, brightest, out_file);

    Texture lambertian = Texture();
    lambertian.src = out_file;
    lambertian.type = radiance.type;
    lambertian.format = radiance.format;
    lambertian.textureImageFormat = radiance.textureImageFormat;
    lambertian.LoadTextureData();

    return std::move(lambertian);
}
