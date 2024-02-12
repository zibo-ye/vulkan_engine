#include "EngineCore.hpp"
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

#pragma comment(lib, "runtimeobject.lib")

namespace EngineCore {
bool gIsSupending = false;

void InitializeApplication(IApp& game)
{
    int argc = 0;
    // LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    // CommandLineArgs::Initialize(argc, argv);

    // Graphics::Initialize(game.RequiresRaytracingSupport());
    // SystemTime::Initialize();
    // GameInput::Initialize();
    // EngineTuning::Initialize();

    game.Startup();
}

void TerminateApplication(IApp& game)
{
    // g_CommandManager.IdleGPU();

    game.Cleanup();

    // GameInput::Shutdown();
}

bool UpdateApplication(IApp& game)
{
    // EngineProfiling::Update();

    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    // float DeltaTime = Graphics::GetFrameTime();

    // GameInput::Update(DeltaTime);
    // EngineTuning::Update(DeltaTime);
    //
    game.Update(DeltaTime);
    game.RenderScene();

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

    return !game.IsDone();
}

// Default implementation to be overridden by the application
bool IApp::IsDone(void)
{
    return false;
    // return GameInput::IsFirstPressed(GameInput::kKey_escape);
}

int IApp::RegisterEventHandler(EIOInputType eventType, std::function<void(EngineCore::IOInput)> handler)
{
    static int handlerId = 0;
    handlerId++;
    eventHandlers[eventType].emplace_back(handlerId, handler);
    return handlerId;
}

void IApp::RemoveEventHandler(int eventType, int handlerId)
{
    auto& handlers = eventHandlers[eventType];
    auto it = std::remove_if(handlers.begin(), handlers.end(),
        [handlerId](const auto& pair) { return pair.first == handlerId; });
    handlers.erase(it, handlers.end());
}

void IApp::RemoveAllEventHandlersForType(int eventType)
{
    eventHandlers.erase(eventType);
}

uint32_t g_DisplayWidth = 800;
uint32_t g_DisplayHeight = 600;

#if USE_GLFW
static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(window));
    app->events.windowResized = true;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(window));
    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::KEYBOARD,
        .key = key,
        .scancode = scancode,
        .action = action,
        .mods = mods,
    });
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(window));
    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::MOUSE_MOVE,
        .x = xpos,
        .y = ypos,
    });
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::MOUSE_BUTTON,
        .x = xpos,
        .y = ypos,
        .button = button,
        .action = action,
        .mods = mods,
    });
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto app = reinterpret_cast<IApp*>(glfwGetWindowUserPointer(window));
    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::MOUSE_SCROLL,
        .x = xoffset,
        .y = yoffset,
    });
}

int RunApplication(IApp&& app, const char* className)
{
    // init window
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // to not create an OpenGL context
    GLFWwindow* window = glfwCreateWindow(
        g_DisplayWidth, // width
        g_DisplayHeight, // height
        className, // window title
        nullptr, // monitor
        nullptr // share
    );
    glfwSetWindowUserPointer(window, &app);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    // bind window
    app.bindGLFWWindow(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // init
    InitializeApplication(app);

    // main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        UpdateApplication(app);
    }

    // cleanup
    TerminateApplication(app);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
#endif

#if USE_NATIVE_WINDOWS_API
HWND g_hWnd = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int RunApplication(IApp&& app, const char* className)
{
    HINSTANCE hInst = GetModuleHandle(nullptr);

    // init window
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

    auto hr = RegisterClassEx(&wcex);
    // ASSERT(0 != hr, "Unable to register a window");

    // Create window
    RECT rc = { 0, 0, (LONG)g_DisplayWidth, (LONG)g_DisplayHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    g_hWnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);

    // ASSERT(g_hWnd != 0);

    // bind window
    app.bindHWND(g_hWnd);

    // init
    InitializeApplication(app);
    ShowWindow(g_hWnd, SW_SHOWDEFAULT);

    // main loop
    do {
        MSG msg = {};
        bool done = false;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                done = true;
        }

        if (done)
            break;
    } while (UpdateApplication(app)); // Returns false to quit loop

    // cleanup
    TerminateApplication(app);
    // Graphics::Shutdown();
    return 0;
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_SIZE:
        // Display::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
#endif

}
