#define _CRT_SECURE_NO_WARNINGS
#include "main.hpp"
#include "Scene/Scene.hpp"

CREATE_APPLICATION(HelloTriangleApplication)

void HelloTriangleApplication::Startup(void)
{
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Articulation.s72");
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Containment.s72" );
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Grouping.s72"    );
    std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sg-Support.s72");
    // std::string scenePath("D:\\dev\\Vulkan\\s72\\examples\\sphereflake.s72"    );

    auto pScene = Scene::loadSceneFromFile(scenePath);
    pScene->PrintStatistics();
    pScene->Traverse();
    m_VulkanCore.Init(this);
}

void HelloTriangleApplication::Cleanup(void)
{
    m_VulkanCore.Shutdown();
}

void HelloTriangleApplication::Update(float deltaT)
{
    ProcessEvents();
}

void HelloTriangleApplication::RenderScene(void)
{
    m_VulkanCore.drawFrame();
}

void HelloTriangleApplication::ProcessEvents()
{
    while (!events.IOInputs.empty()) {
        auto& ioInput = events.IOInputs.front();
        int eventType = ioInput.key; // Or however you determine the event type from an IOInput

        // Check if there are any handlers registered for this event type
        auto handlersIt = eventHandlers.find(eventType);
        if (handlersIt != eventHandlers.end()) {
            // Call each handler with the ioInput event
            for (auto& handler : handlersIt->second) {
                handler.second(ioInput);
            }
        }

        events.IOInputs.pop();
    }
}
