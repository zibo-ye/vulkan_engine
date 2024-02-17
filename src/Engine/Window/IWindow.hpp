#pragma once

#include "EngineCore.hpp"
#include "pch.hpp"

class IWindow : public std::enable_shared_from_this<IWindow> {
public:
    virtual void Create(const std::string& title, int width, int height, EngineCore::IApp& app) = 0;
    virtual void Destroy() = 0;
    virtual void Update() = 0;
    virtual bool ShouldClose() = 0;
    virtual void GetWindowSize(int& width, int& height) const = 0;
    virtual VkSurfaceKHR CreateSurface(VkInstance instance) = 0; // For Vulkan surface creation
    virtual ~IWindow() = default;
    virtual bool IsHeadless() const = 0;
};
