#include "Material.hpp"
#include "SceneEnum.hpp"
#include "Texture.hpp"

Material::Material(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::MATERIAL)
{
    name = jsonObj["name"].getString();

    if (jsonObj.hasKey("normalMap")) {
        normalMap = Texture(jsonObj["normalMap"], pScene.lock()->src);
    }

    if (jsonObj.hasKey("displacementMap")) {
        displacementMap = Texture(jsonObj["displacementMap"], pScene.lock()->src);
    }

    if (jsonObj.hasKey("pbr")) {
        type = EMaterialType::PBR;
        pbr = PBR();

        if (jsonObj["pbr"].hasKey("albedo")) {
            auto albedo = jsonObj["pbr"]["albedo"];
            if (albedo.isArray()) {
                auto vec = albedo.getVecFloat();
                pbr->albedo = vkm::vec3(vec[0], vec[1], vec[2]); // #TODO: generate a texture from pure color
            } else {
                pbr->albedoMap = Texture(albedo, pScene.lock()->src);
            }
        }

        if (jsonObj["pbr"].hasKey("roughness")) {
            auto roughness = jsonObj["pbr"]["roughness"];
            if (roughness.isFloat()) {
                pbr->roughness = roughness.getFloat();
            } else {
                pbr->roughnessMap = Texture(roughness, pScene.lock()->src);
            }
        }

        if (jsonObj["pbr"].hasKey("metalness")) {
            auto metalness = jsonObj["pbr"]["metalness"];
            if (metalness.isFloat()) {
                pbr->metalness = metalness.getFloat();
            } else {
                pbr->metalnessMap = Texture(metalness, pScene.lock()->src);
            }
        }
    } else if (jsonObj.hasKey("lambertian")) {
        type = EMaterialType::LAMBERTIAN;
        lambertian = Lambertian();

        if (jsonObj["lambertian"].hasKey("albedo")) {
            auto albedo = jsonObj["lambertian"]["albedo"];
            if (albedo.isArray()) {
                auto vec = albedo.getVecFloat();
                lambertian->albedo = vkm::vec3(vec[0], vec[1], vec[2]); // #TODO: generate a texture from pure color
            } else {
                lambertian->albedoMap = Texture(albedo, pScene.lock()->src);
            }
        }
    } else if (jsonObj.hasKey("mirror")) {
        type = EMaterialType::MIRROR;
    } else if (jsonObj.hasKey("environment")) {
        type = EMaterialType::ENVIRONMENT;
    } else if (jsonObj.hasKey("simple")) {
        type = EMaterialType::SIMPLE;
    }
}