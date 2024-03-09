#pragma once
#include "../Engine/EngineCore.hpp"
#include "../Engine/Graphics/Vulkan/VulkanCore.hpp"
#include "../Engine/Graphics/Vulkan/VulkanHelper.hpp"
#include "../Engine/pch.hpp"

class MainApplication : public EngineCore::IApp {

public:
    void ParseArguments(const Utility::ArgsParser& argsParser) override;

    void Startup(void) override;

    void Cleanup(void) override;

    void Update(float deltaT) override;

    void RenderScene(void) override;

    void Measure();

    std::pair<int, int> GetWindowSize() override { return args.windowSize; };

    void PresentImage() override;

    void SetPlaybackTimeAndRate(float playbackTime, float playbackRate) override;

    void SaveFrame(std::string savePath) override;

    std::shared_ptr<Scene> GetScene() { return m_Scene; };

private:
    void ProcessEvents();

private:
    VulkanCore m_VulkanCore;
    std::shared_ptr<Scene> m_Scene;
};
