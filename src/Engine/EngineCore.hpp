#pragma once

#include "IO/IOInput.hpp"
#include "pch.hpp"
class IWindow;

namespace EngineCore {
extern bool gIsSupending;

class IApp {
public:
    virtual void ParseArguments(const Utility::ArgsParser& argsParser) = 0;
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

    virtual std::pair<int, int> GetWindowSize() = 0;

    void bindWindow(std::shared_ptr<IWindow> window)
    {
        info.window = window;
    }

    virtual void PresentImage() = 0;
    virtual void SetPlaybackTimeAndRate(float playbackTime, float playbackRate) = 0;
    virtual void SaveFrame(std::string savePath) = 0;

public:
    struct IAppInfo {
        std::shared_ptr<IWindow> window;
    } info;

    struct IAppEvent {
        bool windowResized = false;
        std::queue<IOInput> IOInputs;
    } events;

protected:
    std::unordered_map<EIOInputType, std::vector<std::pair<int, std::function<void(IOInput)>>>> eventHandlers;

public:
    // Returns the handler ID
    int RegisterEventHandler(EIOInputType eventType, std::function<void(IOInput)> handler);

    void RemoveEventHandler(EIOInputType eventType, int handlerId);

    void RemoveAllEventHandlersForType(EIOInputType eventType);

public:
    struct ApplicationArgs {
        std::string scenePath;
        std::optional<std::string> cameraName;
        std::optional<std::string> physicalDeviceName;
        std::pair<int, int> windowSize = { 800, 600 };
        std::optional<std::string> cullingType;
        std::optional<std::string> headlessEventsPath;
        bool measureFPS = false;
        bool limitFPS = false;
        bool profiling = false;
    } args;
};
}

namespace EngineCore {
int RunApplication(IApp&& app, const char* className, const Utility::ArgsParser& args);
}

#define CREATE_APPLICATION(app_class)                                  \
    int main(int argc, const char* argv[])                             \
    {                                                                  \
        try {                                                          \
            auto args = Utility::ArgsParser(argc, argv);               \
            EngineCore::RunApplication(app_class(), #app_class, args); \
        } catch (const std::exception& e) {                            \
            std::cerr << e.what() << std::endl;                        \
            return EXIT_FAILURE;                                       \
        }                                                              \
                                                                       \
        return EXIT_SUCCESS;                                           \
    }