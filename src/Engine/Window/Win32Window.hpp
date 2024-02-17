#pragma once
#if USE_NATIVE_WINDOWS_API
#include "IWindow.hpp"
#include "pch.hpp"
#include <Windows.h>
#include <memory>

namespace EngineCore {
class IApp;
}

class Win32Window : public IWindow {
public:
    void Create(const std::string& title, int width, int height, EngineCore::IApp& app) override;
    void Destroy() override;
    void Update() override;
    bool ShouldClose() override;
    void GetWindowSize(int& width, int& height) const override;
    VkSurfaceKHR CreateSurface(VkInstance instance) override;


 bool IsHeadless() const override {
		return false;
	}
    
private:
    HWND hwnd = nullptr;
    bool shouldClose = false;
    EngineCore::IApp* m_pApp = nullptr;

    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};
#endif