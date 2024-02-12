#pragma once

#include "pch.hpp"

namespace EngineCore {
extern bool gIsSupending;

enum EIOInputType {
    KEYBOARD,
    MOUSE_BUTTON,
    MOUSE_MOVE,
    MOUSE_SCROLL,
};

struct IOInput {
    EIOInputType type;
    std::optional<double> x, y;
    std::optional<int> button;
    std::optional<int> key;
    std::optional<int> scancode;
    std::optional<int> action;
    std::optional<int> mods;
};

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

#if USE_NATIVE_WINDOWS_API
public:
    void bindHWND(HWND hwnd) { info.m_hwnd = hwnd; }
#endif
#if USE_GLFW
public:
    void bindGLFWWindow(GLFWwindow* window) { info.m_pGLFWWindow = window; }
#endif

public:
    struct IAppInfo {
#if USE_NATIVE_WINDOWS_API
        HWND m_hwnd;
#endif
#if USE_GLFW
        GLFWwindow* m_pGLFWWindow = nullptr;
#endif
    } info;

    struct IAppEvent {
        bool windowResized = false;
        std::queue<IOInput> IOInputs;
    } events;

protected:
    std::unordered_map<int, std::vector<std::pair<int, std::function<void(EngineCore::IOInput)>>>> eventHandlers;

public:
    // Returns the handler ID
    int RegisterEventHandler(EIOInputType eventType, std::function<void(EngineCore::IOInput)> handler);

    void RemoveEventHandler(int eventType, int handlerId);

    void RemoveAllEventHandlersForType(int eventType);
};
}

namespace EngineCore {
int RunApplication(IApp&& app, const char* className);
}

#define CREATE_APPLICATION(app_class)                            \
    int main()                                                   \
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