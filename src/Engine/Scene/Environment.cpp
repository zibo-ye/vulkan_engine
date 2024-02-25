#include "Environment.hpp"

Environment::Environment(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::ENVIRONMENT)
{
    name = jsonObj["name"].getString();

    radiance = Texture(jsonObj["radiance"]);
}
