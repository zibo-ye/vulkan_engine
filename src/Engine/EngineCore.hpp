#pragma once

#include "pch.hpp"

namespace EngineCore {
extern bool gIsSupending;

class IApp {
public:
    // This function can be used to initialize application state and will run after essential
    // hardware resources are allocated.  Some state that does not depend on these resources
    // should still be initialized in the constructor such as pointers and flags.
    virtual void Startup(void) = 0;
    virtual void Cleanup(void) = 0;

    // Decide if you want the app to exit.  By default, app continues until the 'ESC' key is pressed.
    virtual bool IsDone(void);

    // The update method will be invoked once per frame.  Both state updating and scene
    // rendering should be handled by this method.
    virtual void Update(float deltaT) = 0;

    // Official rendering pass
    virtual void RenderScene(void) = 0;

    // Optional UI (overlay) rendering pass.  This is LDR.  The buffer is already cleared.
    virtual void RenderUI(class GraphicsContext&) {};

    // Override this in applications that use DirectX Raytracing to require a DXR-capable device.
    virtual bool RequiresRaytracingSupport() const { return false; }

#if USE_NATIVE_WINDOWS_API
    virtual void bindHWND(HWND hwnd) = 0;
#endif
#if USE_GLFW
    virtual void bindGLFWWindow(GLFWwindow* window) = 0;
#endif
public:
    bool windowResized = false;
};
}

namespace EngineCore {
#if USE_GLFW
int RunApplication(IApp&& app, const char* className);
#endif
#if USE_NATIVE_WINDOWS_API
int RunApplication(IApp&& app, const char* className, HINSTANCE hInst, int nCmdShow);
#endif
}



#if USE_GLFW
#define CREATE_APPLICATION(app_class)                            \
int main()                                                       \
    {                                                            \
        try {                                                    \
            EngineCore::RunApplication(app_class(), #app_class); \
        } catch (const std::exception& e) {                      \
            std::cerr << e.what() << std::endl;                  \
            return EXIT_FAILURE;                                 \
        }                                                        \
                                                                 \
        return EXIT_SUCCESS;                                     \
    }

#endif
#if USE_NATIVE_WINDOWS_API
#define CREATE_APPLICATION(app_class)                                                                                                 \
    int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPWSTR /*lpCmdLine*/, _In_ int nCmdShow) \
    {                                                                                                                                 \
        try {                                                                                                                         \
            EngineCore::RunApplication(app_class(), #app_class, hInstance, nCmdShow);                                                 \
        } catch (const std::exception& e) {                                                                                           \
            std::cerr << e.what() << std::endl;                                                                                       \
            return EXIT_FAILURE;                                                                                                      \
        }                                                                                                                             \
                                                                                                                                      \
        return EXIT_SUCCESS;                                                                                                          \
    }
#endif

