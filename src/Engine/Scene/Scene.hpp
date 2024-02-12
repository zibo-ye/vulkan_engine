#pragma once
#include "Camera.hpp"
#include "Mesh.hpp"
#include "SceneEnum.hpp"
#include "SceneObj.hpp"
#include "pch.hpp"

class Node;
class Mesh;
struct MeshInstance;
class Driver;
class Scene;

class Node : public SceneObj {
public:
    Node(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);

    std::string name;
    glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
    glm::quat rotation = { 0.0f, 0.0f, 0.0f, 1.0f }; // Quaternion
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    std::optional<int> meshIdx;
    std::optional<int> cameraIdx;
    std::vector<int> childrenIdx;

    glm::mat4 GetTransform() const;
    void Traverse(glm::mat4 transform, std::vector<MeshInstance>& meshInsts);
};

class Driver : public SceneObj {
public:
    Driver(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj);
    std::string name;
    int nodeIdx;
    EDriverChannelType channel;
    std::vector<float> times;
    std::vector<float> values;
    EDriverInterpolationType interpolation = LINEAR;

    std::optional<std::vector<float>> GetValue(float time) const;
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
    std::unordered_map<size_t, std::shared_ptr<SceneCamera>> cameras;
    std::unordered_map<size_t, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<size_t, std::shared_ptr<Driver>> drivers;

    static std::shared_ptr<Scene> loadSceneFromFile(const std::string& path);
    void PrintStatistics() const;
    static std::shared_ptr<Scene> defaultScene();

    void RegisterEventHandlers(EngineCore::IApp* pApp);

public:
    void Update(float deltaTime);
    // #TODO: don't have to traverse and update all meshInstances every frame, only the ones that have changed, the rest can be cached
    void Traverse(std::vector<MeshInstance>& meshInsts);

public:
    std::unordered_map<size_t, std::vector<size_t>> nodeParents;
    std::unordered_map<size_t, std::vector<size_t>> activeDrivers;
    float m_minDriverLoopTime = std::numeric_limits<float>::min();

private:
    float m_elapsedTime = 0.0f;
    bool m_isLooping = true;
    bool m_isPlaying = true;
};
