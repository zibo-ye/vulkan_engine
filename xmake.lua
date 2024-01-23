set_project("15672_vulkan_engine")

set_warnings("all")
set_languages("c++20")

add_rules("mode.debug","mode.releasedbg", "mode.release")

add_requires("vulkansdk", "glfw", "glm")

add_requires("glslang", {configs = {binaryonly = true}})

target("Engine")
    set_kind("static")
    add_packages("vulkansdk", "glfw", "glm")
    -- set_pcheader("src/Engine/pch.hpp")
    add_headerfiles("src/Engine/**.hpp")
    add_includedirs("src/Engine")
    add_files("src/Engine/**.cpp")


target("Triangle")
    set_default(true)
    set_kind("binary")
    add_packages("vulkansdk", "glfw", "glm")
    -- set_pcheader("src/Engine/pch.hpp")
    add_headerfiles("src/Triangle/**.hpp")
    add_rules("utils.glsl2spv", {outputdir = "shader_build"})
    add_includedirs("src/Engine")
    add_files("src/Triangle/**.cpp")
    add_files("src/Triangle/shader/*.vert", "src/Triangle/shader/*.frag")
    add_deps("Engine")
    set_rundir("$(projectdir)")
