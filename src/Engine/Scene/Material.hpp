#pragma once
#include "Scene.hpp"
#include "SceneObj.hpp"
#include "Texture.hpp"
#include "pch.hpp"

class Material : public SceneObj {
public:
    Material(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);

    std::string name;

    std::optional<Texture> normalMap;
    std::optional<Texture> displacementMap;

    EMaterialType type = EMaterialType::SIMPLE;

    // PBR properties
    struct PBR {
        Texture albedoMap = Texture(vkm::vec3(1.0f, 1.0f, 1.0f));
        Texture roughnessMap = Texture(1.0f);
        Texture metalnessMap = Texture(0.0f);
    };
    std::optional<PBR> pbr;

    // Lambertian properties
    struct Lambertian {
        Texture albedoMap = Texture(vkm::vec3(1.0f, 1.0f, 1.0f));
    };
    std::optional<Lambertian> lambertian;
};