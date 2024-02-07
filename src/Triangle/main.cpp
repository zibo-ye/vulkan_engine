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
}

void HelloTriangleApplication::RenderScene(void)
{
    m_VulkanCore.drawFrame();
}
