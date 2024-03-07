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
        pbr = PBR();

        if (jsonObj["pbr"].hasKey("albedo")) {
            auto vec = jsonObj["pbr"]["albedo"].getVecFloat();
            pbr->albedo = vkm::vec3(vec[0], vec[1], vec[2]);
        }

        if (jsonObj["pbr"].hasKey("roughness")) {
            auto vec = jsonObj["pbr"]["roughness"].getVecFloat();
            pbr->roughness = vec[0];
        }

        if (jsonObj["pbr"].hasKey("metalness")) {
            auto vec = jsonObj["pbr"]["metalness"].getVecFloat();
            pbr->metalness = vec[0];
        }

        if (jsonObj["pbr"].hasKey("albedoMap")) {
            pbr->albedoMap = Texture(jsonObj["pbr"]["albedoMap"], pScene.lock()->src);
        }

        if (jsonObj["pbr"].hasKey("roughnessMap")) {
            pbr->roughnessMap = Texture(jsonObj["pbr"]["roughnessMap"], pScene.lock()->src);
        }

        if (jsonObj["pbr"].hasKey("metalnessMap")) {
            pbr->metalnessMap = Texture(jsonObj["pbr"]["metalnessMap"], pScene.lock()->src);
        }
    }

    if (jsonObj.hasKey("lambertian")) {
        lambertian = Lambertian();

        if (jsonObj["lambertian"].hasKey("baseColor")) {
            auto vec = jsonObj["lambertian"]["baseColor"].getVecFloat();
            lambertian->baseColor = vkm::vec3(vec[0], vec[1], vec[2]);
        }

        if (jsonObj["lambertian"].hasKey("baseColorMap")) {
            lambertian->baseColorMap = Texture(jsonObj["lambertian"]["baseColorMap"], pScene.lock()->src);
        }
    }

    isMirror = jsonObj.hasKey("mirror");
    isEnvironment = jsonObj.hasKey("environment");
    isSimple = jsonObj.hasKey("simple");
}