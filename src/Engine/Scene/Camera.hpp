#pragma once
#include "SceneObj.hpp"
#include "pch.hpp"

class Scene;
class Mesh;

enum ECameraType {
    EScene,
    EUser
};

class ICamera {
public:
    virtual ~ICamera() = default;
    virtual ECameraType getType() const = 0;
    virtual std::string getName() const = 0;
    virtual void update(float deltaTime) = 0;
    virtual vkm::mat4 getViewMatrix() const = 0;
    virtual vkm::mat4 getProjectionMatrix() const = 0;
    virtual bool FrustumCulling(std::shared_ptr<Mesh> pMesh, vkm::mat4& worldTransform);
};

struct Perspective {
    float aspect = 800.f / 600.f;
    float vfov = vkm::radians(45.f);
    float near_plane = 0.1f;
    float far_plane = 10.f;
    vkm::mat4 getProjectionMatrix() const
    {
        return vkm::perspective(vfov, aspect, near_plane, far_plane);
    }
};

class SceneCamera : public SceneObj, public ICamera {
public:
    SceneCamera(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
        : SceneObj(pScene, index, ESceneObjType::CAMERA)
    {
        name = jsonObj["name"].getString();

        if (jsonObj.getObject().find("perspective") != jsonObj.getObject().end()) {
            auto& persp = jsonObj["perspective"];
            perspective = {
                .aspect = persp["aspect"].getFloat(),
                .vfov = persp["vfov"].getFloat(),
                .near_plane = persp["near"].getFloat(),
                .far_plane = persp["far"].getFloat()
            };
        }
    }

    ECameraType getType() const override { return ECameraType::EScene; }
    std::string getName() const override { return name; }

    void update(float deltaTime) override { }

    vkm::mat4 getViewMatrix() const override;

    vkm::mat4 getProjectionMatrix() const override
    {
        return perspective.getProjectionMatrix();
    }

    // Existing properties and methods from your original Camera class
    std::string name;
    Perspective perspective;
};

struct UserCameraUpdateParameters {
    std::optional<vkm::vec3> fromPos;
    std::optional<vkm::vec3> lookAtPos;
    std::optional<float> aspect;
    std::optional<float> vfov;
    std::optional<float> near_plane;
    std::optional<float> far_plane;
};

class UserCamera : public ICamera {
public:
    UserCamera(std::string name)
        : name(name) {};
    ECameraType getType() const override { return ECameraType::EUser; }
    std::string getName() const override { return name; }

    void update(float deltaTime) override
    {
    }

    vkm::mat4 getViewMatrix() const override
    {
        return vkm::lookAt(fromPos, lookAtPos, vkm::vec3(0.0f, 0.0f, 1.0f));
    }

    vkm::mat4 getProjectionMatrix() const override
    {
        return perspective.getProjectionMatrix();
    }

    void UpdateCameraParameters(UserCameraUpdateParameters params);

    std::string name;
    vkm::vec3 fromPos = vkm::vec3(2.0f, 2.0f, 2.0f);
    vkm::vec3 lookAtPos = vkm::vec3(0.0f, 0.0f, 0.0f);
    Perspective perspective;
};
