#pragma once
#include "../Engine/pch.hpp"
#include "../Engine/EngineCore.hpp"
#include "../Engine/Graphics/Vulkan/VulkanCore.hpp"

class HelloTriangleApplication : public EngineCore::IApp {

public:
    void Startup(void) override;

    void Cleanup(void) override;

    void Update(float deltaT) override;

    void RenderScene(void) override;

private:
    VulkanCore m_VulkanCore;
};
