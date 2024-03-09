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
};
Texture GenerateLambertian(Texture radiance, int lambertian_texWidth = 16);