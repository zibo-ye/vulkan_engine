
#include "CameraManager.hpp"
#include "EngineCore.hpp"
#include "Scene.hpp"
#include "IO/IOInput.hpp"

void CameraManager::Init(EngineCore::IApp* pApp)
{
    // Add a default user camera
    std::string name = "UserCamera";
    auto userCamera = std::make_shared<UserCamera>(name);
    userCamera->UpdateCameraParameters({ .aspect = float(pApp->args.windowSize.first) / float(pApp->args.windowSize.second) });
    CameraManager::GetInstance().AddCamera(name, userCamera);

#ifndef NDEBUG
    debugCamera = std::make_shared<UserCamera>("DebugCamera");
    debugCamera->UpdateCameraParameters({ .aspect = float(pApp->args.windowSize.first) / float(pApp->args.windowSize.second) });
#endif

    if (pApp->args.cameraName)
    {
        CameraManager::GetInstance().SetActiveCamera(pApp->args.cameraName.value());
    }
    else
    {
        CameraManager::GetInstance().SetActiveCamera(userCamera->name);
	}

    // Register event handlers
    // #TODO: Support allocating different keys

    // Tab: Switch between cameras
    pApp->RegisterEventHandler(EIOInputType::KEYBOARD, [this](IOInput input) {
        if (input.key == EKeyboardKeys::TAB && input.action == EKeyAction::PRESS)
            this->SwitchToNextCamera();
    });

    // W, A, S, D, Q, E, R: Move the user camera
    pApp->RegisterEventHandler(EIOInputType::KEYBOARD, [this](IOInput input) {
        // EKeyAction::REPEAT only works after a delay, so event won't be triggered immediately, see CameraManager::UpdateCamera()
        keysActivated[input.key.value()] = (input.action == EKeyAction::PRESS || input.action == EKeyAction::REPEAT); // no EKeyAction::RELEASE means activated.

#ifndef NDEBUG
        if (input.key == EKeyboardKeys::F5 && input.action == EKeyAction::PRESS) {
            isDebugCameraActive = !isDebugCameraActive;
            std::cout << "Debug Camera: " << (isDebugCameraActive ? "Active" : "Inactive") << "\n";
        }
#endif
    });

    // Mouse drag: Rotate the user camera
    pApp->RegisterEventHandler(EIOInputType::MOUSE_BUTTON, [this](IOInput input) {
        auto camera = activeCamera;
#ifndef NDEBUG
        if (IsDebugModeActive())
            camera = GetDebugCamera();
#endif
        if (camera->getType() == ECameraType::EUser && input.button == EMouseButton::LEFT) {
            if (input.action == EKeyAction::PRESS) {
                // When the left mouse button is pressed, record the current mouse position
                lastMousePos = glm::vec2(input.x.value(), input.y.value());
                isMouseLeftButtonDown = true;
            } else if (input.action == EKeyAction::RELEASE) {
                // Reset the last mouse position when the button is released
                lastMousePos = glm::vec2(-1, -1);
                isMouseLeftButtonDown = false;
            }
        }
    });

    pApp->RegisterEventHandler(EIOInputType::MOUSE_MOVE, [this](IOInput input) {
        auto camera = activeCamera;
#ifndef NDEBUG
        if (IsDebugModeActive())
            camera = GetDebugCamera();
#endif
        if (camera->getType() == ECameraType::EUser && isMouseLeftButtonDown) {
            if (lastMousePos.x >= 0 && lastMousePos.y >= 0) {
                UserCamera* userCamera = static_cast<UserCamera*>(camera.get());
                glm::vec2 currentMousePos = glm::vec2(input.x.value(), input.y.value());
                glm::vec2 mouseDelta = currentMousePos - lastMousePos;
                // std::cout << "Mouse Delta: " << mouseDelta.x << ", " << mouseDelta.y << "\n";
                lastMousePos = currentMousePos;

                constexpr float vertical_angle_limit = glm::radians(80.0f); //+-80 degrees
                constexpr float vertical_angle_limit_max = glm::radians(90.0f) + vertical_angle_limit;
                constexpr float vertical_angle_limit_min = glm::radians(90.0f) - vertical_angle_limit;
                constexpr float vertical_delta_limit = glm::radians(90.0f) - vertical_angle_limit;

                float horizAngle = mouseSensitivity * mouseDelta.x * -1;
                float vertAngle = mouseSensitivity * mouseDelta.y * -1;
                vertAngle = std::clamp(vertAngle, -vertical_delta_limit, vertical_delta_limit);

                glm::vec3 upDir = glm::vec3(0.0f, 0.0f, 1.0f);
                glm::vec3 lookAtDir = glm::normalize(userCamera->lookAtPos - userCamera->fromPos);
                glm::vec3 right = glm::normalize(glm::cross(lookAtDir, upDir));

                // Horizontal rotation
                glm::mat4 horizRot = glm::rotate(glm::mat4(1.0f), horizAngle, upDir);
                glm::vec3 horizontalDirection = glm::mat3(horizRot) * lookAtDir;

                // Vertical rotation
                glm::mat4 vertRot = glm::rotate(glm::mat4(1.0f), vertAngle, right);
                glm::vec3 tentativeDirection = glm::mat3(vertRot) * horizontalDirection;

                // Calculate the angle between the tentative direction and the up vector
                float currentAngle = std::acos(glm::dot(glm::normalize(tentativeDirection), upDir));

                // If the angle exceeds the limit, adjust vertical angle to meet the limit
                if (currentAngle > vertical_angle_limit_max || currentAngle < vertical_angle_limit_min) {
                    vertAngle = 0.0f; // remove the vertical rotation
                }
                vertRot = glm::rotate(glm::mat4(1.0f), vertAngle, right);
                glm::vec3 finalDirection = glm::mat3(vertRot) * horizontalDirection;

                userCamera->fromPos = userCamera->lookAtPos - finalDirection * glm::distance(userCamera->lookAtPos, userCamera->fromPos);
            }
        }
    });

    // Mouse Scroll: Zoom in/out the user camera
    pApp->RegisterEventHandler(EIOInputType::MOUSE_SCROLL, [this](IOInput input) {
        auto camera = activeCamera;
#ifndef NDEBUG
        if (IsDebugModeActive())
            camera = GetDebugCamera();
#endif
        if (camera->getType() == ECameraType::EUser) {
            UserCamera* userCamera = static_cast<UserCamera*>(camera.get());
            float zoomSpeed = 0.1f;
            // userCamera->fov -= input.yoffset * zoomSpeed; //TODO: clamp

            glm::vec3 lookAtDir = userCamera->lookAtPos - userCamera->fromPos;
            auto yoffset = input.y.value();
            userCamera->fromPos += lookAtDir * static_cast<float>(yoffset) * zoomSpeed;
        }
    });
}

void CameraManager::UpdateCamera(float deltaTime)
{
    auto camera = activeCamera;
#ifndef NDEBUG
    if (IsDebugModeActive())
        camera = GetDebugCamera();
#endif
    // Update the active camera position when keysActivated WASDQE are pressed, or X -> Reset
    if (camera->getType() == ECameraType::EUser) {
        UserCamera* userCamera = static_cast<UserCamera*>(camera.get());

        auto lookAtDir = userCamera->lookAtPos - userCamera->fromPos;
        lookAtDir = glm::normalize(lookAtDir);

        auto upDir = glm::vec3(0.0f, 0.0f, 1.0f);
        auto right = glm::cross(lookAtDir, upDir);

        float movingSpeed = keyboardMovingSpeed * deltaTime;

        if (keysActivated[EKeyboardKeys::W]) {
            auto right = glm::cross(lookAtDir, upDir);
            auto screenUp = glm::cross(right, lookAtDir);
            userCamera->fromPos += screenUp * movingSpeed;
            userCamera->lookAtPos += screenUp * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::S]) {
            auto right = glm::cross(lookAtDir, upDir);
            auto screenUp = glm::cross(right, lookAtDir);
            userCamera->fromPos -= screenUp * movingSpeed;
            userCamera->lookAtPos -= screenUp * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::A]) {
            auto right = glm::cross(lookAtDir, upDir);
            right = glm::normalize(right);
            userCamera->fromPos -= right * movingSpeed;
            userCamera->lookAtPos -= right * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::D]) {
            auto right = glm::cross(lookAtDir, upDir);
            right = glm::normalize(right);
            userCamera->fromPos += right * movingSpeed;
            userCamera->lookAtPos += right * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::Q]) {
            userCamera->fromPos += lookAtDir * movingSpeed;
            userCamera->lookAtPos += lookAtDir * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::E]) {
            userCamera->fromPos -= lookAtDir * movingSpeed;
            userCamera->lookAtPos -= lookAtDir * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::X]) {
            *userCamera = UserCamera(userCamera->name);
        }
    }
}
