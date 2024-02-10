#pragma once
#include "SceneEnum.hpp"
#include "pch.hpp"

class Scene;

class SceneObj {
public:
    SceneObj(std::weak_ptr<Scene> pScene, size_t index, ESceneObjType type)
        : pScene(pScene)
        , index(index)
        , type(type) {};

    ESceneObjType type;
    std::weak_ptr<Scene> pScene;
    size_t index;
};
