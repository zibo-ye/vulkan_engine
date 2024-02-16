#pragma once
#include "IWindow.hpp"
#include "pch.hpp"

#if USE_GLFW
class GLFWWindow : public IWindow {
public:
    GLFWwindow* window = nullptr;

    virtual void Create(const std::string& title, int width, int height, EngineCore::IApp& app) override;
    virtual void Destroy() override;
    virtual void Update() override;
    virtual bool ShouldClose() override;
    virtual void GetWindowSize(int& width, int& height) const override;
    virtual VkSurfaceKHR CreateSurface(VkInstance instance) override;
};
#endif