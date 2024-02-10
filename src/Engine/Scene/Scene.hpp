#pragma once
#include "SceneEnum.hpp"
#include "SceneObj.hpp"
#include "Mesh.hpp"
#include "pch.hpp"

class Node;
class Mesh;
class Camera;
class Driver;
class Scene;

class Node : public SceneObj {
public:
    Node(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);

    std::string name;
    std::vector<float> translation = { 0.0f, 0.0f, 0.0f };
    std::vector<float> rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; // Quaternion
    std::vector<float> scale = { 1.0f, 1.0f, 1.0f };
    std::optional<int> meshIdx;
    std::optional<int> cameraIdx;
    std::vector<int> childrenIdx;

    void Traverse();
};

class Camera : public SceneObj {
public:
    Camera(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);

    std::string name;
    struct Perspective {
        float aspect;
        float vfov;
        float near_plane;
        float far_plane;
    } perspective;
    std::vector<int> parents; // To track the node hierarchy
};

class Driver : public SceneObj {
public:
    Driver(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);
    std::string name;
    int nodeIdx;
    EDriverChannelType channel;
    std::vector<float> times;
    std::vector<float> values;
    EDriverInterpolationType interpolation;
};

class Scene : public std::enable_shared_from_this<Scene> {
public:
    Scene() = default;
    void Init(const Utility::json::JsonValue& jsonObj);

    std::string name = "";
    std::string src = "";
    std::vector<int> roots;

    std::unordered_map<size_t, std::shared_ptr<SceneObj>> sceneObjs;
    std::unordered_map<size_t, std::shared_ptr<Node>> nodes;
    std::unordered_map<size_t, std::shared_ptr<Camera>> cameras;
    std::unordered_map<size_t, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<size_t, std::shared_ptr<Driver>> drivers;

    static std::shared_ptr<Scene> loadSceneFromFile(const std::string& path);
    void PrintStatistics() const;
    static std::shared_ptr<Scene> defaultScene();

public:
    void Traverse();

private:
};
