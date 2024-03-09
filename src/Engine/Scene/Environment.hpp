#pragma once
#include "Scene.hpp"
#include "SceneObj.hpp"
#include "Texture.hpp"
#include "pch.hpp"

class Environment : public SceneObj {
public:
    std::string name;
    Texture radiance;
    Texture lambertian;

    Environment(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);

    void generateCubemaps(VulkanCore* pVulkanCore);
    Texture irradiance;
    Texture preFilteredEnv;
};
Texture GenerateLambertian(Texture radiance, int lambertian_texWidth = 16);
Texture GenerateLambertian2(Texture radiance);
