#include "Shader.hpp"

Shader::Shader(VkDevice device, const std::string& filename, VkShaderStageFlagBits stage)
    : m_device(device)
    , m_shaderStage(stage)
{
    // Construct the full path to the shader file
    std::filesystem::path cwd = std::filesystem::current_path();
    std::filesystem::path shaderPath = cwd / "shader_build" / filename;

    // Read the shader code from the file
    auto shaderCode = readShaderFile(shaderPath);

    // Create the shader module
    m_shaderModule = createShaderModule(shaderCode);
}

Shader::~Shader()
{
    if (m_shaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(m_device, m_shaderModule, nullptr);
    }
}

VkShaderModule Shader::createShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
