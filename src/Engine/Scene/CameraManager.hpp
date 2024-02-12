#pragma once
#include "pch.hpp"
#include "Camera.hpp"

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

                #if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
                std::cout << "Switched to camera: " << activeCamera->getName() << std::endl;
				#endif
			}
		}
	}



private:
    // Private constructor to prevent instantiation outside of getInstance()
    CameraManager() = default;

    std::unordered_map<std::string, std::shared_ptr<ICamera>> cameras;
    std::shared_ptr<ICamera> activeCamera;

    std::unordered_map<int, bool> keysActivated;
    float keyboardMovingSpeed = 0.1f;

    float mouseSensitivity = glm::radians(1.0f);
    glm::vec2 lastMousePos;
    bool isMouseLeftButtonDown = false;
};
