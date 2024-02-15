#define _CRT_SECURE_NO_WARNINGS
#include "main.hpp"
#include "Scene/CameraManager.hpp"
#include "Scene/Scene.hpp"

// #TODO: change name
CREATE_APPLICATION(MainApplication)

void MainApplication::Startup(void)
{
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Articulation.s72");
    // std::string scenePath("/Users/immmortal/dev/s72/examples/sg-Articulation.s72");
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Containment.s72");
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Grouping.s72"); //Animation
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Support.s72");
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sphereflake.s72");

    m_Scene = Scene::loadSceneFromFile(scenePath);
    m_Scene->RegisterEventHandlers(this);
    m_Scene->PrintStatistics();

    CameraManager::GetInstance().Init(this);
    m_VulkanCore.Init(this);
}

void MainApplication::Cleanup(void)
{
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
    m_VulkanCore.drawFrame(*m_Scene);
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
