#pragma once
#include "Graphics/Vulkan/VulkanCore.hpp"
#include "pch.hpp"

class Shader {
public:
    Shader(VkDevice device, const std::string& filename, VkShaderStageFlagBits stage);

    ~Shader();

    VkPipelineShaderStageCreateInfo getShaderStageInfo() const
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = m_shaderStage,
            .module = m_shaderModule,
            .pName = "main",
        };
		return shaderStageInfo;
	}

private:
    VkDevice m_device;
    VkShaderModule m_shaderModule;
    VkShaderStageFlagBits m_shaderStage;

    VkShaderModule createShaderModule(const std::vector<char>& code);

};
