#include "GLFWWindow.hpp"
#if USE_GLFW

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<EngineCore::IApp*>(glfwGetWindowUserPointer(window));
    app->events.windowResized = true;

    //Handling minimization
    int new_width = 0, new_height = 0;
    glfwGetFramebufferSize(window, &new_width, &new_height);
    while (new_width == 0 || new_height == 0) {
        glfwGetFramebufferSize(window, &new_width, &new_height);
        glfwWaitEvents();
    }
}

EKeyAction GLFWKeyAction(int action)
{
    switch (action) {
	case GLFW_PRESS:
		return EKeyAction::PRESS;
        case GLFW_RELEASE:
		return EKeyAction::RELEASE;
        case GLFW_REPEAT:
		return EKeyAction::REPEAT;
	default:
		return EKeyAction::UNKNOWN;
	}
}

EMouseButton GLFWMouseButton(int button)
{
    switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		return EMouseButton::LEFT;
	case GLFW_MOUSE_BUTTON_RIGHT:
		return EMouseButton::RIGHT;
	case GLFW_MOUSE_BUTTON_MIDDLE:
		return EMouseButton::MIDDLE;
	default:
		return EMouseButton::LEFT;
	}
}

EKeyboardKeys GLFWKey(int key)
{
	switch (key) {
	case GLFW_KEY_W:
		return EKeyboardKeys::W;
	case GLFW_KEY_S:
		return EKeyboardKeys::S;
	case GLFW_KEY_A:
		return EKeyboardKeys::A;
	case GLFW_KEY_D:
		return EKeyboardKeys::D;
	case GLFW_KEY_Q:
		return EKeyboardKeys::Q;
	case GLFW_KEY_E:
		return EKeyboardKeys::E;
	case GLFW_KEY_R:
		return EKeyboardKeys::R;
	case GLFW_KEY_X:
		return EKeyboardKeys::X;
	case GLFW_KEY_UP:
		return EKeyboardKeys::UP;
	case GLFW_KEY_DOWN:
		return EKeyboardKeys::DOWN;
	case GLFW_KEY_LEFT:
		return EKeyboardKeys::LEFT;
	case GLFW_KEY_RIGHT:
		return EKeyboardKeys::RIGHT;
	case GLFW_KEY_SPACE:
		return EKeyboardKeys::SPACE;
	case GLFW_KEY_TAB:
		return EKeyboardKeys::TAB;
	case GLFW_KEY_F5:
		return EKeyboardKeys::F5;
	default:
		return EKeyboardKeys::UNKNOWN;
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    auto app = reinterpret_cast<EngineCore::IApp*>(glfwGetWindowUserPointer(window));
    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::KEYBOARD,
        .key = GLFWKey(key),
        .scancode = scancode,
        .action = GLFWKeyAction(action),
        .mods = mods,
    });
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto app = reinterpret_cast<EngineCore::IApp*>(glfwGetWindowUserPointer(window));
    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::MOUSE_MOVE,
        .x = xpos,
        .y = ypos,
    });
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto app = reinterpret_cast<EngineCore::IApp*>(glfwGetWindowUserPointer(window));
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::MOUSE_BUTTON,
        .x = xpos,
        .y = ypos,
        .button = GLFWMouseButton(button),
        .action = GLFWKeyAction(action),
        .mods = mods,
    });
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto app = reinterpret_cast<EngineCore::IApp*>(glfwGetWindowUserPointer(window));
    app->events.IOInputs.push(IOInput {
        .type = EIOInputType::MOUSE_SCROLL,
        .x = xoffset,
        .y = yoffset,
    });
}


void GLFWWindow::Create(const std::string& title, int width, int height, EngineCore::IApp& app)
{
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No OpenGL context
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(window, &app);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
}
void GLFWWindow::Destroy()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void GLFWWindow::Update()
{
	glfwPollEvents();
}

bool GLFWWindow::ShouldClose()
{
	return glfwWindowShouldClose(window);
}

void GLFWWindow::GetWindowSize(int& width, int& height) const
{
	glfwGetWindowSize(window, &width, &height);
}

VkSurfaceKHR GLFWWindow::CreateSurface(VkInstance instance)
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface");
	}
	return surface;
}
#endif