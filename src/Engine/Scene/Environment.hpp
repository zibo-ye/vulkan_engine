#pragma once
#include "Scene.hpp"
#include "SceneObj.hpp"
#include "pch.hpp"
#include "Texture.hpp"

class Environment : public SceneObj {
public:
    std::string name;
    Texture radiance;

    Environment(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);
};