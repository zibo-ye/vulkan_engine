#if USE_NATIVE_WINDOWS_API
#include "Win32Window.hpp"
#include "EngineCore.hpp"
#include <stdexcept>
#include <vulkan/vulkan_win32.h> // Include this for Vulkan surface creation on Win32

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

std::optional<VkSurfaceKHR> Win32Window::CreateSurface(VkInstance instance)
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

EKeyboardKeys TranslateKey(int key)
{
    switch (key) {
    case 'W':
        return EKeyboardKeys::W;
    case 'S':
        return EKeyboardKeys::S;
    case 'A':
        return EKeyboardKeys::A;
    case 'D':
        return EKeyboardKeys::D;
    case 'Q':
        return EKeyboardKeys::Q;
    case 'E':
        return EKeyboardKeys::E;
    case 'R':
        return EKeyboardKeys::R;
    case 'X':
        return EKeyboardKeys::X;
    case 'L':
        return EKeyboardKeys::L;
    case VK_UP:
        return EKeyboardKeys::UP;
    case VK_DOWN:
        return EKeyboardKeys::DOWN;
    case VK_LEFT:
        return EKeyboardKeys::LEFT;
    case VK_RIGHT:
        return EKeyboardKeys::RIGHT;
    case VK_SPACE:
        return EKeyboardKeys::SPACE;
    case VK_TAB:
        return EKeyboardKeys::TAB;
    case VK_F5:
        return EKeyboardKeys::F5;
    default:
        return EKeyboardKeys::UNKNOWN;
    }
    return EKeyboardKeys::UNKNOWN;
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
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        window->m_pApp->events.windowResized = true;

        // Handling minimization (width and height become 0)
        if (width == 0 || height == 0) {
            // Optionally, handle minimization or pausing of rendering here
        }
    } break;
    case WM_KEYDOWN:
    case WM_KEYUP: {
        int key = static_cast<int>(wParam); // Virtual-Key Code
        EKeyAction action = (message == WM_KEYDOWN) ? EKeyAction::PRESS : EKeyAction::RELEASE;

        window->m_pApp->events.IOInputs.push(IOInput {
            .type = EIOInputType::KEYBOARD,
            .key = TranslateKey(key),
            .action = action,
        });
    } break;
    case WM_MOUSEMOVE: {
        double xpos = LOWORD(lParam);
        double ypos = HIWORD(lParam);

        window->m_pApp->events.IOInputs.push(IOInput {
            .type = EIOInputType::MOUSE_MOVE,
            .x = xpos,
            .y = ypos,
        });
    } break;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP: {
        EMouseButton button = (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP) ? EMouseButton::LEFT : EMouseButton::RIGHT;
        EKeyAction action = (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN) ? EKeyAction::PRESS : EKeyAction::RELEASE;
        // Getting cursor position
        WORD xpos = LOWORD(lParam);
        WORD ypos = HIWORD(lParam);
        window->m_pApp->events.IOInputs.push(IOInput {
            .type = EIOInputType::MOUSE_BUTTON,
            .x = static_cast<double>(xpos),
            .y = static_cast<double>(ypos),
            .button = button,
            .action = action,
        });
    } break;
    case WM_MOUSEWHEEL: {
        auto zDelta = GET_WHEEL_DELTA_WPARAM(wParam); // Wheel rotation
        double yoffset = zDelta / WHEEL_DELTA; // Wheel delta is defined as 120

        window->m_pApp->events.IOInputs.push(IOInput {
            .type = EIOInputType::MOUSE_SCROLL,
            .y = yoffset,
        });
    } break;

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
#endif