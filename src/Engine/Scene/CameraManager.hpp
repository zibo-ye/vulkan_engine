#pragma once
#include "../Main/main.hpp"
#include "Camera.hpp"
#include "pch.hpp"

namespace EngineCore {
class IApp;
}

class CameraManager {
public:
    // Deletes copy constructor and assignment operator to prevent copying
    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;

    // Provides global access to the singleton instance
    static CameraManager& GetInstance()
    {
        static CameraManager instance;
        return instance;
    }

    void Init(EngineCore::IApp* pApp);

    void UpdateCamera(float deltaTime);

    void AddCamera(const std::string& name, std::shared_ptr<ICamera> camera)
    {
        cameras[name] = camera;
    }

    // #TODO: removeCamera

    void SetActiveCamera(const std::string& name)
    {
        auto it = cameras.find(name);
        if (it != cameras.end()) {
            activeCamera = it->second;
        }
    }

    void Update(float deltaTime)
    {
        UpdateCamera(deltaTime);

        if (activeCamera) {
            activeCamera->update(deltaTime);
        }
    }

    std::shared_ptr<ICamera> GetActiveCamera() const
    {
        return activeCamera;
    }

    void SwitchToNextCamera()
    {
        if (cameras.size() > 1) {
            auto it = cameras.find(activeCamera->getName());
            if (it != cameras.end()) {
                ++it;
                if (it == cameras.end()) {
                    it = cameras.begin();
                }
                activeCamera = it->second;

#ifndef NDEBUG
                std::cout << "Switched to camera: " << activeCamera->getName() << std::endl;
#endif
            }
        }
    }

#ifndef NDEBUG
    bool IsDebugModeActive() const
    {
        return isDebugCameraActive;
    }

    std::shared_ptr<UserCamera> GetDebugCamera() const
    {
        return debugCamera;
    }
#endif

private:
    // Private constructor to prevent instantiation outside of getInstance()
    CameraManager() = default;

    std::unordered_map<std::string, std::shared_ptr<ICamera>> cameras;
    std::shared_ptr<ICamera> activeCamera;

#ifndef NDEBUG
    std::shared_ptr<UserCamera> debugCamera;
    bool isDebugCameraActive = false;
#endif

    std::unordered_map<EKeyboardKeys, bool> keysActivated;
    float keyboardMovingSpeed = 10.0f;

    float mouseSensitivity = vkm::radians(1.0f);
    vkm::vec2 lastMousePos;
    bool isMouseLeftButtonDown = false;
};
