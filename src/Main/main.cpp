#define _CRT_SECURE_NO_WARNINGS
#include "main.hpp"
#include "Scene/CameraManager.hpp"
#include "Scene/Scene.hpp"

CREATE_APPLICATION(MainApplication)

void MainApplication::ParseArguments(const Utility::ArgsParser& argsParser)
{
    if (argsParser.GetArg("list-physical-devices")) {
        printAllPhysicalDevices();
        exit(0);
    }
    if (argsParser.GetArg("list-instance-extensions")) {
        printAllAvailableInstanceExtensions();
        exit(0);
    }

    // #TODO : Implement these
    // if (argsParser.GetArg("list-device-extensions")) {
    //     printAllAvailableDeviceExtensions();
    //     exit(0);
    // }

    // if (argsParser.GetArg("get-device-info")) {
    //    exit(0);
    //}

    auto sceneArg = argsParser.GetArg("scene");
    if (sceneArg.has_value()) {
        args.scenePath = sceneArg.value()[0];
    } else {
        throw std::runtime_error("No scene file provided.");
    }

    auto cameraArg = argsParser.GetArg("camera");
    if (cameraArg.has_value()) {
        args.cameraName = cameraArg.value()[0];
    }

    auto physicalDeviceArg = argsParser.GetArg("physical-device");
    if (physicalDeviceArg.has_value()) {
        args.physicalDeviceName = physicalDeviceArg.value()[0];
    }

    auto windowSizeArg = argsParser.GetArg("drawing-size");
    if (windowSizeArg.has_value()) {
        args.windowSize.first = std::stoi(windowSizeArg.value()[0]);
        args.windowSize.second = std::stoi(windowSizeArg.value()[1]);
    }

    auto cullingTypeArg = argsParser.GetArg("culling");
    if (cullingTypeArg.has_value()) {
        args.cullingType = cullingTypeArg.value()[0];
    }

    auto headlessEventsPathArg = argsParser.GetArg("headless");
    if (headlessEventsPathArg.has_value()) {
        args.headlessEventsPath = headlessEventsPathArg.value()[0];
    }

    auto measureFPSArg = argsParser.GetArg("measure");
    if (measureFPSArg.has_value()) {
        args.measureFPS = true;
    }

    auto limitFPSArg = argsParser.GetArg("limitfps");
    if (limitFPSArg.has_value()) {
        args.limitFPS = true;
    }
    auto profilingArg = argsParser.GetArg("profiling");
    if (profilingArg.has_value()) {
        args.profiling = true;
    }
}

void MainApplication::Startup(void)
{
    m_Scene = Scene::loadSceneFromFile(args.scenePath);
    m_Scene->RegisterEventHandlers(this);
    m_Scene->PrintStatistics();

    CameraManager::GetInstance().Init(this);
    m_VulkanCore.Init(this);
}

void MainApplication::Cleanup(void)
{
    m_VulkanCore.WaitIdle();
    m_Scene->Cleanup();
    m_VulkanCore.Shutdown();
}

void MainApplication::Update(float deltaT)
{
    ProcessEvents();
    CameraManager::GetInstance().Update(deltaT);
    m_Scene->Update(deltaT);
}

void MainApplication::RenderScene(void)
{
    if (args.measureFPS) {
        static int frameCount = 0;
        // static auto startTime = std::chrono::high_resolution_clock::now();
        static auto lastOutputTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastOutputTime).count();
        if (args.headlessEventsPath) {
            if (m_VulkanCore.readyForNextImage || !args.limitFPS) {
                frameCount++;
            }
        } else {
            frameCount++;
        }
        if (DeltaTime >= 1.0f) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastOutputTime = currentTime;
        }
    }
    m_VulkanCore.drawFrame(*m_Scene);
}

void MainApplication::PresentImage()
{
    m_VulkanCore.PresentImage();
}

void MainApplication::SetPlaybackTimeAndRate(float playbackTime, float playbackRate)
{
    m_Scene->SetPlaybackTimeAndRate(playbackTime, playbackRate);
}

void MainApplication::SaveFrame(std::string savePath)
{
    if (args.profiling) {
        // std::cerr << "Profiling is enabled. Ignoring save frame request." << std::endl;
        return;
    }
    m_VulkanCore.SaveFrame(savePath);
}

void MainApplication::ProcessEvents()
{
    while (!events.IOInputs.empty()) {
        auto& ioInput = events.IOInputs.front();

        auto handlersIt = eventHandlers.find(ioInput.type);
        if (handlersIt != eventHandlers.end()) {
            for (auto& handler : handlersIt->second) {
                handler.second(ioInput);
            }
        }

        events.IOInputs.pop();
    }
}
