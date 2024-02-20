
#include "CameraManager.hpp"
#include "EngineCore.hpp"
#include "IO/IOInput.hpp"
#include "Scene.hpp"

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

    if (pApp->args.cameraName) {
        CameraManager::GetInstance().SetActiveCamera(pApp->args.cameraName.value());
    } else {
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
                lastMousePos = vkm::vec2(static_cast<float>(input.x.value()), static_cast<float>(input.y.value()));
                isMouseLeftButtonDown = true;
            } else if (input.action == EKeyAction::RELEASE) {
                // Reset the last mouse position when the button is released
                lastMousePos = vkm::vec2(-1, -1);
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
            if (lastMousePos.x() >= 0 && lastMousePos.y() >= 0) {
                UserCamera* userCamera = static_cast<UserCamera*>(camera.get());
                vkm::vec2 currentMousePos = vkm::vec2(static_cast<float>(input.x.value()), static_cast<float>(input.y.value()));
                vkm::vec2 mouseDelta = currentMousePos - lastMousePos;
                // std::cout << "Mouse Delta: " << mouseDelta.x << ", " << mouseDelta.y << "\n";
                lastMousePos = currentMousePos;

                constexpr float vertical_angle_limit = vkm::radians(80.0f); //+-80 degrees
                constexpr float vertical_angle_limit_max = vkm::radians(90.0f) + vertical_angle_limit;
                constexpr float vertical_angle_limit_min = vkm::radians(90.0f) - vertical_angle_limit;
                constexpr float vertical_delta_limit = vkm::radians(90.0f) - vertical_angle_limit;

                float horizAngle = mouseSensitivity * mouseDelta.x() * -1;
                float vertAngle = mouseSensitivity * mouseDelta.y() * -1;
                vertAngle = std::clamp(vertAngle, -vertical_delta_limit, vertical_delta_limit);

                vkm::vec3 upDir = vkm::vec3(0.0f, 0.0f, 1.0f);
                vkm::vec3 lookAtDir = userCamera->lookAtPos - userCamera->fromPos;
                lookAtDir = vkm::normalize(lookAtDir);
                vkm::vec3 right = vkm::normalize(vkm::cross(lookAtDir, upDir));

                // Horizontal rotation
                vkm::mat4 horizRot = vkm::rotate(vkm::mat4(1.0f), horizAngle, upDir);
                vkm::vec3 horizontalDirection = vkm::mat3(horizRot) * lookAtDir;

                // Vertical rotation
                vkm::mat4 vertRot = vkm::rotate(vkm::mat4(1.0f), vertAngle, right);
                vkm::vec3 tentativeDirection = vkm::mat3(vertRot) * horizontalDirection;

                // Calculate the angle between the tentative direction and the up vector
                float currentAngle = std::acos(vkm::dot(vkm::normalize(tentativeDirection), upDir));

                // If the angle exceeds the limit, adjust vertical angle to meet the limit
                if (currentAngle > vertical_angle_limit_max || currentAngle < vertical_angle_limit_min) {
                    vertAngle = 0.0f; // remove the vertical rotation
                }
                vertRot = vkm::rotate(vkm::mat4(1.0f), vertAngle, right);
                vkm::vec3 finalDirection = vkm::mat3(vertRot) * horizontalDirection;

                userCamera->fromPos = userCamera->lookAtPos - finalDirection * vkm::distance(userCamera->lookAtPos, userCamera->fromPos);
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

            vkm::vec3 lookAtDir = userCamera->lookAtPos - userCamera->fromPos;
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

        vkm::vec3 lookAtDir = userCamera->lookAtPos - userCamera->fromPos;
        lookAtDir = vkm::normalize(lookAtDir);

        auto upDir = vkm::vec3(0.0f, 0.0f, 1.0f);
        auto right = vkm::cross(lookAtDir, upDir);

        float movingSpeed = keyboardMovingSpeed * deltaTime;

        if (keysActivated[EKeyboardKeys::W]) {
            auto right = vkm::cross(lookAtDir, upDir);
            auto screenUp = vkm::cross(right, lookAtDir);
            userCamera->fromPos += screenUp * movingSpeed;
            userCamera->lookAtPos += screenUp * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::S]) {
            auto right = vkm::cross(lookAtDir, upDir);
            auto screenUp = vkm::cross(right, lookAtDir);
            userCamera->fromPos -= screenUp * movingSpeed;
            userCamera->lookAtPos -= screenUp * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::A]) {
            auto right = vkm::cross(lookAtDir, upDir);
            right = vkm::normalize(right);
            userCamera->fromPos -= right * movingSpeed;
            userCamera->lookAtPos -= right * movingSpeed;
        } else if (keysActivated[EKeyboardKeys::D]) {
            auto right = vkm::cross(lookAtDir, upDir);
            right = vkm::normalize(right);
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
