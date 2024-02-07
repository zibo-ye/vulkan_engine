#define _CRT_SECURE_NO_WARNINGS
#include "main.hpp"

CREATE_APPLICATION(HelloTriangleApplication)

void HelloTriangleApplication::Startup(void)
{
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
