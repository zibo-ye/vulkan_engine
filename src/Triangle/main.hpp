#pragma once
#include "../Engine/EngineCore.hpp"
#include "../Engine/Graphics/Vulkan/VulkanCore.hpp"
#include "../Engine/pch.hpp"

class HelloTriangleApplication : public EngineCore::IApp {

public:
    void Startup(void) override;

    void Cleanup(void) override;

    void Update(float deltaT) override;

    void RenderScene(void) override;

private:
    void ProcessEvents();

private:
    VulkanCore m_VulkanCore;
    std::shared_ptr<Scene> m_Scene;
};
