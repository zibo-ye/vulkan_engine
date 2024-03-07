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

    // PBR properties
    struct PBR {
        vkm::vec3 albedo = { 1.0f, 1.0f, 1.0f };
        float roughness = 1.0f;
        float metalness = 0.0f;
        std::optional<Texture> albedoMap;
        std::optional<Texture> roughnessMap;
        std::optional<Texture> metalnessMap;
    };
    std::optional<PBR> pbr;

    // Lambertian properties
    struct Lambertian {
        vkm::vec3 baseColor = { 1.0f, 1.0f, 1.0f };
        std::optional<Texture> baseColorMap;
    };
    std::optional<Lambertian> lambertian;

    // Other material types
    bool isMirror = false;
    bool isEnvironment = false;
    bool isSimple = false;
};