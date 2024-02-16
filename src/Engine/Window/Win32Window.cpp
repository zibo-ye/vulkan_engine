#include "Win32Window.hpp"
#include <stdexcept>
#include <vulkan/vulkan_win32.h> // Include this for Vulkan surface creation on Win32
#include "EngineCore.hpp"

// Create window
void Win32Window::Create(const std::string& title, int width, int height, EngineCore::IApp& app)
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    m_pApp = &app;

    // Define window class
    WNDCLASSEX wc = {
        .cbSize = sizeof(WNDCLASSEX),
        .style = CS_HREDRAW | CS_VREDRAW,
        .lpfnWndProc = WindowProc,
        .hInstance = hInstance,
        .hCursor = LoadCursor(nullptr, IDC_ARROW),
        .hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
        .lpszClassName = "Win32WindowClass",
    };

    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error("Failed to register window class");
    }

    // Create window
    hwnd = CreateWindowEx(0, wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) {
        throw std::runtime_error("Failed to create window");
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
}

void Win32Window::Destroy()
{
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
    UnregisterClass("Win32WindowClass", GetModuleHandle(nullptr));
}

void Win32Window::Update()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

bool Win32Window::ShouldClose()
{
    return shouldClose;
}

void Win32Window::GetWindowSize(int& width, int& height) const
{
    RECT rect;
    if (GetClientRect(hwnd, &rect)) {
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }
}

VkSurfaceKHR Win32Window::CreateSurface(VkInstance instance)
{
    VkSurfaceKHR surface;

    VkWin32SurfaceCreateInfoKHR createInfo {
        .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(nullptr),
        .hwnd = hwnd
    };
    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }

    return surface;
}

LRESULT CALLBACK Win32Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message) {
    //case WM_SIZE: {
    //    int width = LOWORD(lParam);
    //    int height = HIWORD(lParam);

    //    m_pApp->events.windowResized = true;

    //    // Handling minimization (width and height become 0)
    //    if (width == 0 || height == 0) {
    //        // Optionally, handle minimization or pausing of rendering here
    //    }
    //} break;
    //case WM_KEYDOWN:
    //case WM_KEYUP: {
    //    int key = wParam; // Virtual-Key Code
    //    int action = (message == WM_KEYDOWN) ? 1 : 0; // 1 for press, 0 for release

    //     m_pApp->events.IOInputs.push(IOInput {
    //        .type = EIOInputType::KEYBOARD,
    //        .key = key,
    //        .action = action,
    //    });
    //} break;
    //case WM_MOUSEMOVE: {
    //    double xpos = LOWORD(lParam);
    //    double ypos = HIWORD(lParam);

    //    m_pApp->events.IOInputs.push(IOInput {
    //        .type = EIOInputType::MOUSE_MOVE,
    //        .x = xpos,
    //        .y = ypos,
    //    });
    //} break;
    //case WM_LBUTTONDOWN:
    //case WM_LBUTTONUP:
    //case WM_RBUTTONDOWN:
    //case WM_RBUTTONUP:
    //    // Add cases for other buttons as needed
    //    {
    //        int button = (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) ? 0 : 1; // Example for left (0) and right (1) buttons
    //        int action = (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN) ? 1 : 0; // 1 for press, 0 for release
    //        // Getting cursor position
    //        POINTS pt = MAKEPOINTS(lParam);
    //        double xpos = pt.x;
    //        double ypos = pt.y;

    //        m_pApp->events.IOInputs.push(IOInput {
    //            .type = EIOInputType::MOUSE_BUTTON,
    //            .x = xpos,
    //            .y = ypos,
    //            .button = button,
    //            .action = action,
    //            // 'mods' handling might need additional logic
    //        });
    //    }
    //    break;
    //case WM_MOUSEWHEEL: {
    //    auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // Wheel rotation
    //    double yoffset = zDelta / WHEEL_DELTA; // Wheel delta is defined as 120

    //    m_pApp->events.IOInputs.push(IOInput {
    //        .type = EIOInputType::MOUSE_SCROLL,
    //        // xoffset is typically 0 for vertical scroll, yoffset for the scroll amount
    //        .y = yoffset,
    //    });
    //} break;


    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_CLOSE:
        window->shouldClose = true;
        PostQuitMessage(0);
        return 0;
        // Handle other messages here as needed
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}