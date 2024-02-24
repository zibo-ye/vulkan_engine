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

    auto physicalDeviceArg = argsParser.GetArg("physical-device");
    if (physicalDeviceArg.has_value()) {
        args.physicalDeviceName = physicalDeviceArg.value()[0];
    }

    if (argsParser.GetArg("list-device-extensions")) {
        if (args.physicalDeviceName)
            printAllAvailableDeviceExtensions(*args.physicalDeviceName);
        else
            throw std::runtime_error("No physical-device provided.");
        exit(0);
    }

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

    auto measureArg = argsParser.GetArg("measure");
    if (measureArg.has_value()) {
        args.measure = true;
    }

    auto limitFPSArg = argsParser.GetArg("limitfps");
    if (limitFPSArg.has_value()) {
        args.limitFPS = true;
    }
    auto headlessIgnoreSaveFrameArg = argsParser.GetArg("headlessIgnoreSaveFrame");
    if (headlessIgnoreSaveFrameArg.has_value()) {
        args.headlessIgnoreSaveFrame = true;
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
    if (args.measure)
        Measure();
    m_VulkanCore.drawFrame(*m_Scene);
}

void MainApplication::Measure()
{
    static std::vector<float> frameTimes;
    static auto lastFrameTime = std::chrono::high_resolution_clock::now();
    static auto lastOutputTime = std::chrono::high_resolution_clock::now();
    static float deltaOutputTime = 0.0f;
    auto currentTime = std::chrono::high_resolution_clock::now();
    float frameTimeInMicrosec = std::chrono::duration<float, std::chrono::microseconds::period>(currentTime - lastFrameTime).count();
    deltaOutputTime += frameTimeInMicrosec;

    if (args.headlessEventsPath) {
        if (m_VulkanCore.readyForNextImage || !args.limitFPS) {
            frameTimes.push_back(frameTimeInMicrosec);
        }
    } else {
        frameTimes.push_back(frameTimeInMicrosec);
    }

    // Calculate statistics every second or every N frames
    if (deltaOutputTime >= 1000000.f) { // N is the number of frames after which you want to calculate statistics
        float averageFrameTime = deltaOutputTime / frameTimes.size();
        float fps = 1000000.0f / averageFrameTime;

        // Sort frame times for percentile calculations
        std::sort(frameTimes.begin(), frameTimes.end());
        float p99 = frameTimes.at(std::lround(frameTimes.size() * 0.99) - 1);
        float p95 = frameTimes.at(std::lround(frameTimes.size() * 0.95) - 1);
        float p90 = frameTimes.at(std::lround(frameTimes.size() * 0.90) - 1);

        // Calculate standard deviation
        float sumOfSquaredDifferences = std::accumulate(frameTimes.begin(), frameTimes.end(), 0.0f,
            [averageFrameTime](float acc, float ft) { return acc + (ft - averageFrameTime) * (ft - averageFrameTime); });
        float std_dev = std::sqrt(sumOfSquaredDifferences / frameTimes.size());

        std::cout << "FPS: " << fps << ", Avg Frame Time: " << averageFrameTime
                  << "us , P99: " << p99 << "us , P95: " << p95 << "us , P90: " << p90
                  << "us , Std Dev: " << std_dev << std::endl;

        frameTimes.clear(); // Reset for next batch
        deltaOutputTime = 0.0f;
    }
    lastFrameTime = std::chrono::high_resolution_clock::now(); // To eliminate the time taken to print statistics from the next batch
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
    if (args.headlessIgnoreSaveFrame) {
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
