#include "EngineCore.hpp"
#include "IO/IOInput.hpp"
#include "pch.hpp"

// #include "GraphicsCore.hpp"
// #include "SystemTime.hpp"
// #include "GameInput.hpp"
// #include "BufferManager.hpp"
// #include "CommandContext.hpp"
// #include "PostEffects.hpp"
// #include "Display.hpp"
// #include "Util/CommandLineArg.hpp"
// #include <shellapi.h>

#include "Window/GLFWWindow.hpp"
#include "Window/Win32Window.hpp"
#include "Window/HeadlessWindow.hpp"

#pragma comment(lib, "runtimeobject.lib")

namespace EngineCore {
bool gIsSupending = false;

void InitializeApplication(IApp& app)
{
    int argc = 0;
    // LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    // CommandLineArgs::Initialize(argc, argv);

    // Graphics::Initialize(game.RequiresRaytracingSupport());
    // SystemTime::Initialize();
    // GameInput::Initialize();
    // EngineTuning::Initialize();

    app.Startup();
}

void TerminateApplication(IApp& app)
{
    // g_CommandManager.IdleGPU();

    app.Cleanup();

    // GameInput::Shutdown();
}

bool UpdateApplication(IApp& app)
{
    // EngineProfiling::Update();

    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
    lastTime = currentTime;
    float ElapsedTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    // float DeltaTime = Graphics::GetFrameTime();

    // GameInput::Update(DeltaTime);
    // EngineTuning::Update(DeltaTime);
    //
    app.Update(DeltaTime);
    app.RenderScene();

    // PostEffects::Render();

    // GraphicsContext& UiContext = GraphicsContext::Begin(L"Render UI");
    // UiContext.TransitionResource(g_OverlayBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    // UiContext.ClearColor(g_OverlayBuffer);
    // UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
    // UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
    // game.RenderUI(UiContext);

    // UiContext.SetRenderTarget(g_OverlayBuffer.GetRTV());
    // UiContext.SetViewportAndScissor(0, 0, g_OverlayBuffer.GetWidth(), g_OverlayBuffer.GetHeight());
    // EngineTuning::Display( UiContext, 10.0f, 40.0f, 1900.0f, 1040.0f );

    // UiContext.Finish();

    // Display::Present();

    return !app.IsDone();
}

// Default implementation to be overridden by the application
bool IApp::IsDone(void)
{
    return false;
    // return GameInput::IsFirstPressed(GameInput::kKey_escape);
}

int IApp::RegisterEventHandler(EIOInputType eventType, std::function<void(IOInput)> handler)
{
    static int handlerId = 0;
    handlerId++;
    eventHandlers[eventType].emplace_back(handlerId, handler);
    return handlerId;
}

void IApp::RemoveEventHandler(EIOInputType eventType, int handlerId)
{
    auto& handlers = eventHandlers[eventType];
    auto it = std::remove_if(handlers.begin(), handlers.end(),
        [handlerId](const auto& pair) { return pair.first == handlerId; });
    handlers.erase(it, handlers.end());
}

void IApp::RemoveAllEventHandlersForType(EIOInputType eventType)
{
    eventHandlers.erase(eventType);
}

std::shared_ptr<IWindow> CreateIWindow(std::optional<std::string> headlessEventsPath)
{
    if (headlessEventsPath.has_value()) {
        std::cout << "Creating headless window with events path: " << headlessEventsPath.value() << std::endl;
		return std::make_shared<HeadlessWindow>(headlessEventsPath.value());
	}
#if USE_GLFW
    return std::make_shared<GLFWWindow>();
#elif USE_NATIVE_WINDOWS_API
    return std::make_shared<Win32Window>();
#else
#error "Unsupported window system"
#endif
}

int RunApplication(IApp&& app, const char* className, const Utility::ArgsParser& args)
{
    app.ParseArguments(args);

    auto windowSize = app.GetWindowSize();
    std::shared_ptr<IWindow> window = CreateIWindow(app.args.headlessEventsPath);
    window->Create(className, windowSize.first, windowSize.second, app);

    app.bindWindow(window);
    InitializeApplication(app);

    while (!window->ShouldClose()) {
        window->Update();
        UpdateApplication(app);
    }

    TerminateApplication(app);
    window->Destroy();

    return 0;
}
}
