#pragma once

#define USE_NATIVE_API 0
#define USE_VULKAN 1

#define USE_GLFW !USE_NATIVE_API
#define USE_GLM 1
// #define USE_GLM !USE_NATIVE_API
#define USE_NATIVE_WINDOWS_API (_WIN32 && USE_NATIVE_API)

#define VERBOSE 1

#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable : 4239) // A non-const reference may only be bound to an lvalue; assignment operator takes a reference to non-const
#pragma warning(disable : 4324) // structure was padded due to __declspec(align())

#if USE_GLFW

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#if _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#endif

#if USE_NATIVE_WINDOWS_API
#include <winsdkver.h>
#define _WIN32_WINNT 0x0A00
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <wrl/client.h>
#include <wrl/event.h>
#endif

#if USE_GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#endif

#include "Math/math.hpp"

#if USE_VULKAN
#if USE_NATIVE_WINDOWS_API
#define VK_USE_PLATFORM_WIN32_KHR
#endif
// #define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.h>
#endif
#include <vulkan/vk_enum_string_helper.h>

#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cwctype>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include <map>
#include <queue>
#include <ranges>
#include <set>

#include <functional>
#include <optional>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

#include "Utility.hpp"
// #include "VectorMath.hpp"
// #include "EngineTuning.hpp"
// #include "EngineProfiling.hpp"
// #include "Util/CommandLineArg.hpp"
