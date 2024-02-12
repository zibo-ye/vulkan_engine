
#include "Scene.hpp"
#include "CameraManager.hpp"

void Scene::Init(const Utility::json::JsonValue& jsonObj)
{
    printf("Hello there!\n");
    if (!jsonObj.isArray())
        throw std::runtime_error("File Format Wrong!");

    auto& array = jsonObj.getArray();
    if (array.size() < 2 || !array[0].isString() || array[0].getString() != "s72-v1")
        throw std::runtime_error("File Format Wrong!");

    for (size_t i = 1; i < array.size(); ++i) {
        auto& val = array[i];
        if (val["type"].getString() == "NODE") {
            auto pNode = std::make_shared<Node>(shared_from_this(), i, val);
            nodes[i] = pNode;
            sceneObjs[i] = pNode;
        } else if (val["type"].getString() == "MESH") {
            auto pMesh = std::make_shared<Mesh>(shared_from_this(), i, val);
            meshes[i] = pMesh;
            sceneObjs[i] = pMesh;
        } else if (val["type"].getString() == "CAMERA") {
            auto pCamera = std::make_shared<SceneCamera>(shared_from_this(), i, val);
            cameras[i] = pCamera;
            sceneObjs[i] = pCamera;
            CameraManager::GetInstance().AddCamera(pCamera->name, pCamera);
        } else if (val["type"].getString() == "DRIVER") {
            auto pDriver = std::make_shared<Driver>(shared_from_this(), i, val);
            drivers[i] = pDriver;
            sceneObjs[i] = pDriver;
        } else if (val["type"].getString() == "SCENE") {
            name = val["name"].getString();
            for (auto& root : val["roots"].getArray()) {
                roots.push_back(root.getInt());
            }
        }
    }

    // #TODO: Update the parentIdx of the nodes
}

std::shared_ptr<Scene> Scene::loadSceneFromFile(const std::string& path)
{
    auto pScene = std::make_shared<Scene>();
    pScene->src = path;
    pScene->Init(Utility::json::JsonValue::parseJsonFromFile(path));
    return pScene;
}

void Scene::PrintStatistics() const
{
    std::cout << "Scene Name: " << name << std::endl;
    std::cout << "Scene Object Count: " << sceneObjs.size() << std::endl;
    std::cout << "Mesh Count: " << meshes.size() << std::endl;
    std::cout << "Node Count: " << nodes.size() << std::endl;
    std::cout << "Camera Count: " << cameras.size() << std::endl;
    std::cout << "Driver Count: " << drivers.size() << std::endl;
    std::cout << "Root Count: " << roots.size() << std::endl;
}

void Scene::Traverse(std::vector<MeshInstance>& meshInsts)
{
    for (auto& root : roots) {
        nodes[root]->Traverse(glm::mat4(1.0f), meshInsts);
    }
}

static std::shared_ptr<Scene> defaultScene()
{
    static std::shared_ptr<Scene> defScene = std::make_shared<Scene>();
    return defScene;
}

Node::Node(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::NODE)
{
    name = jsonObj["name"].getString();

    if (jsonObj.getObject().find("translation") != jsonObj.getObject().end()) {
        auto vec = jsonObj["translation"].getVecFloat();
        translation = glm::vec3(vec[0], vec[1], vec[2]);
    }
    if (jsonObj.getObject().find("rotation") != jsonObj.getObject().end()) {
        auto vec = jsonObj["rotation"].getVecFloat();
        rotation = glm::quat(vec[3], vec[0], vec[1], vec[2]);
    }
    if (jsonObj.getObject().find("scale") != jsonObj.getObject().end()) {
        auto vec = jsonObj["scale"].getVecFloat();
        scale = glm::vec3(vec[0], vec[1], vec[2]);
    }

    if (jsonObj.getObject().find("children") != jsonObj.getObject().end()) {
        for (auto& child : jsonObj["children"].getArray()) {
            auto idx = child.getInt();
            childrenIdx.push_back(idx);
            pScene.lock()->nodeParents[idx].push_back(index);
        }
    }

    if (jsonObj.getObject().find("mesh") != jsonObj.getObject().end()) {
        meshIdx = jsonObj["mesh"].getInt();
        pScene.lock()->nodeParents[meshIdx.value()].push_back(index);
    }
    if (jsonObj.getObject().find("camera") != jsonObj.getObject().end()) {
        cameraIdx = jsonObj["camera"].getInt();
        pScene.lock()->nodeParents[cameraIdx.value()].push_back(index);
    }
}

glm::mat4 Node::GetTransform() const
{
    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::translate(mat, translation);
    mat = mat * glm::mat4_cast(rotation);
    mat = glm::scale(mat, scale);
    return mat;
}

void Node::Traverse(glm::mat4 transform, std::vector<MeshInstance>& meshInst)
{
    if (!meshIdx && childrenIdx.empty()) {
        return;
    }

    auto worldTransform = transform * GetTransform();

    if (meshIdx) {
        auto pMesh = pScene.lock()->meshes[*meshIdx];
        // Frustum Culling
        if (CameraManager::GetInstance().GetActiveCamera()->FrustumCulling(pMesh, worldTransform))
            meshInst.push_back({ pMesh, worldTransform });
    }

    for (auto& child : childrenIdx) {
        if (!pScene.lock())
            break;
        pScene.lock()->nodes[child]->Traverse(worldTransform, meshInst);
    }
}

Driver::Driver(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::DRIVER)
{
    name = jsonObj["name"].getString();
}
