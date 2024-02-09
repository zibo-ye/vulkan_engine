
#include "Scene.hpp"

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
            auto pCamera = std::make_shared<Camera>(shared_from_this(), i, val);
            cameras[i] = pCamera;
            sceneObjs[i] = pCamera;
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
}

std::shared_ptr<Scene> Scene::loadSceneFromFile(const std::string& path)
{
    auto pScene = std::make_shared<Scene>();
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

void Scene::Traverse()
{
    for (auto& root : roots) {

        nodes[root]->Traverse();
    }
}

static std::shared_ptr<Scene> defaultScene()
{
    static std::shared_ptr<Scene> defScene = std::make_shared<Scene>();
    return defScene;
}

Mesh::Mesh(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::MESH)
{
    name = jsonObj["name"].getString();
    topology = GetVkPrimitiveTopology(jsonObj["topology"].getString());
    count = jsonObj["count"].getInt();

    if (jsonObj.getObject().find("indices") != jsonObj.getObject().end()) {
        indices = MeshIndices(jsonObj["indices"]);
    }

    auto& attributes = jsonObj["attributes"];
    for (auto& [attrName, attrVal] : attributes.getObject()) {
        this->attributes[attrName] = MeshAttributes(attrVal);
    }
}

Node::Node(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::NODE)
{
    name = jsonObj["name"].getString();

    if (jsonObj.getObject().find("translation") != jsonObj.getObject().end()) {
        translation = jsonObj["translation"].getVecFloat();
    }
    if (jsonObj.getObject().find("rotation") != jsonObj.getObject().end()) {
        rotation = jsonObj["rotation"].getVecFloat();
    }
    if (jsonObj.getObject().find("scale") != jsonObj.getObject().end()) {
        scale = jsonObj["scale"].getVecFloat();
    }

    if (jsonObj.getObject().find("children") != jsonObj.getObject().end()) {
        for (auto& child : jsonObj["children"].getArray()) {
            childrenIdx.push_back(child.getInt());
        }
    }

    if (jsonObj.getObject().find("mesh") != jsonObj.getObject().end()) {
        meshIdx = jsonObj["mesh"].getInt();
    }
    if (jsonObj.getObject().find("camera") != jsonObj.getObject().end()) {
        cameraIdx = jsonObj["camera"].getInt();
    }
}

void Node::Traverse()
{
    std::cout << "Node: " << name << std::endl;
    for (auto& child : childrenIdx) {
        if (!pScene.lock())
            break;
        pScene.lock()->nodes[child]->Traverse();
    }
}

Camera::Camera(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
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

Driver::Driver(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::DRIVER)
{
    name = jsonObj["name"].getString();
}

MeshIndices::MeshIndices(const Utility::json::JsonValue& jsonObj)
{
    src = jsonObj["src"].getString();
    offset = jsonObj["offset"].getInt();
    format = GetVkFormat(jsonObj["format"].getString());
}

MeshAttributes::MeshAttributes(const Utility::json::JsonValue& jsonObj)
{
    src = jsonObj["src"].getString();
    offset = jsonObj["offset"].getInt();
    stride = jsonObj["stride"].getInt();
    format = GetVkFormat(jsonObj["format"].getString());
}
