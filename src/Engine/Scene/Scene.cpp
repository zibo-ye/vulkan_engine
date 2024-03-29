
#include "Scene.hpp"
#include "Camera.hpp"
#include "CameraManager.hpp"
#include "EngineCore.hpp"
#include "Environment.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

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
        } else if (val["type"].getString() == "MATERIAL") {
            auto pMaterial = std::make_shared<Material>(shared_from_this(), i, val);
            materials[i] = pMaterial;
            sceneObjs[i] = pMaterial;
        } else if (val["type"].getString() == "ENVIRONMENT") {
#if VERBOSE
            if (environment) {
                std::cerr << "Scene::Init: Multiple Environment Objects not supported" << std::endl;
            }
#endif
            environment = std::make_shared<Environment>(shared_from_this(), i, val);
            sceneObjs[i] = environment;
        } else {
            throw std::runtime_error("Unknown Scene Object Type");
        }
    }
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

std::shared_ptr<Scene> Scene::defaultScene()
{
    static std::shared_ptr<Scene> pScene = nullptr;
    if (!pScene)
    {
        pScene = std::make_shared<Scene>();
		std::filesystem::path cwd = std::filesystem::current_path();
		std::filesystem::path scenePath = cwd / "assets" / "Box.s72";
		pScene->src = scenePath.string();
		pScene->Init(Utility::json::JsonValue::parseJsonFromFile(scenePath.string()));
    }
	return pScene;
}

void Scene::RegisterEventHandlers(EngineCore::IApp* pApp)
{
    pApp->RegisterEventHandler(EIOInputType::KEYBOARD, [this](IOInput input) {
        if (input.key == EKeyboardKeys::SPACE && input.action == EKeyAction::PRESS) {
            m_isPlaying = !m_isPlaying;
        }
        if (input.key == EKeyboardKeys::L && input.action == EKeyAction::PRESS) {
            m_isLooping = !m_isLooping;
        }
        if (input.key == EKeyboardKeys::R && input.action == EKeyAction::PRESS) {
            m_elapsedTime = 0.0f;
        }
    });
}

void Scene::Cleanup() const
{
}

void Scene::Update(float deltaTime)
{
    if (m_isPlaying)
        m_elapsedTime += deltaTime * m_PlaybackSpeed;

    float inloopTime = m_elapsedTime;
    if (m_isLooping) {
        m_elapsedTime = fmodf(m_elapsedTime, m_minDriverLoopTime);
    }

    for (auto& [nodeIdx, driverIdxs] : activeDrivers) {
        auto pNode = nodes[nodeIdx];
        for (auto& driverIdx : driverIdxs) {
            auto pDriver = drivers[driverIdx];
            auto value = pDriver->GetValue(inloopTime);
            if (value) {
                // std::cout<< "Updating Node " << pNode->name << " with Driver " << pDriver->name << " at time " << m_elapsedTime << "\n";
                switch (pDriver->channel) {
                case EDriverChannelType::TRANSLATION: {
                    pNode->translation = vkm::vec3(value.value()[0], value.value()[1], value.value()[2]);
                    // std::cout << "Translation: " << value.value()[0] << " " << value.value()[1] << " " << value.value()[2] << "\n";
                    break;
                }
                case EDriverChannelType::ROTATION: {
                    pNode->rotation = vkm::quat(value.value()[0], value.value()[1], value.value()[2], value.value()[3]);
                    // std::cout <<  "Rotation: " << value.value()[0] << " " << value.value()[1] << " " << value.value()[2] << " " << value.value()[3] << "\n";
                    break;
                }
                case EDriverChannelType::SCALE: {
                    pNode->scale = vkm::vec3(value.value()[0], value.value()[1], value.value()[2]);
                    // std::cout <<  "Scale: " << value.value()[0] << " " << value.value()[1] << " " << value.value()[2] << "\n";
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}

void Scene::Traverse(std::vector<MeshInstance>& meshInsts)
{
    for (auto& root : roots) {
        nodes[root]->Traverse(vkm::mat4(), meshInsts);
    }
}

void Scene::SetPlaybackTimeAndRate(float playbackTime, float playbackRate)
{
    m_elapsedTime = playbackTime;
    m_PlaybackSpeed = playbackRate;
}

Node::Node(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::NODE)
{
    name = jsonObj["name"].getString();

    if (jsonObj.hasKey("translation")) {
        auto vec = jsonObj["translation"].getVecFloat();
        translation = vkm::vec3(vec[0], vec[1], vec[2]);
    }
    if (jsonObj.hasKey("rotation")) {
        auto vec = jsonObj["rotation"].getVecFloat();
        rotation = vkm::quat(vec[0], vec[1], vec[2], vec[3]);
    }
    if (jsonObj.hasKey("scale")) {
        auto vec = jsonObj["scale"].getVecFloat();
        scale = vkm::vec3(vec[0], vec[1], vec[2]);
    }

    if (jsonObj.hasKey("children")) {
        for (auto& child : jsonObj["children"].getArray()) {
            auto idx = child.getInt();
            childrenIdx.push_back(idx);
            pScene.lock()->nodeParents[idx].push_back(index);
        }
    }

    if (jsonObj.hasKey("mesh")) {
        meshIdx = jsonObj["mesh"].getInt();
        pScene.lock()->nodeParents[meshIdx.value()].push_back(index);
    }
    if (jsonObj.hasKey("camera")) {
        cameraIdx = jsonObj["camera"].getInt();
        pScene.lock()->nodeParents[cameraIdx.value()].push_back(index);
    }
    if (jsonObj.hasKey("environment")) {
        envIdx = jsonObj["environment"].getInt();
    }
}

vkm::mat4 Node::GetTransform() const
{
    vkm::mat4 mat;
    mat = vkm::translate(mat, translation);
    mat = mat * vkm::mat4_cast(rotation);
    mat = vkm::scale(mat, scale);
    return mat;
}

void Node::Traverse(vkm::mat4 transform, std::vector<MeshInstance>& meshInst)
{
    if (!meshIdx && childrenIdx.empty()) {
        return;
    }

    auto worldTransform = transform * GetTransform();

    if (meshIdx) {
        auto pMesh = pScene.lock()->meshes[*meshIdx];
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
    nodeIdx = jsonObj["node"].getInt();
    channel = GetDriverChannelType(jsonObj["channel"].getString());
    times = jsonObj["times"].getVecFloat();
    values = jsonObj["values"].getVecFloat();
    if (jsonObj.hasKey("interpolation")) {
        interpolation = GetDriverInterpolationType(jsonObj["interpolation"].getString());
    }

    pScene.lock()->activeDrivers[nodeIdx].push_back(index);
    pScene.lock()->m_minDriverLoopTime = std::max(pScene.lock()->m_minDriverLoopTime, *(times.rbegin()));
}

std::optional<std::vector<float>> Driver::GetValue(float time) const
{
    if (times.size() == 0)
        return std::nullopt;

    if (time < times[0])
        return std::nullopt;

    size_t resultSize = values.size() / times.size();

    if (time > times[times.size() - 1]) { // Extrapolation
        return std::vector<float>(values.end() - resultSize, values.end());
    }

    for (size_t i = 0; i < times.size() - 1; ++i) {
        if (time >= times[i] && time <= times[i + 1]) {
            if (interpolation == EDriverInterpolationType::STEP)
                return std::vector<float>(values.begin() + i * resultSize, values.begin() + (i + 1) * resultSize);

            float t = (time - times[i]) / (times[i + 1] - times[i]);
            std::vector<float> value1 = std::vector<float>(values.begin() + i * resultSize, values.begin() + (i + 1) * resultSize);
            std::vector<float> value2 = std::vector<float>(values.begin() + (i + 1) * resultSize, values.begin() + (i + 2) * resultSize);

            if (interpolation == EDriverInterpolationType::LINEAR) {
                std::vector<float> result(resultSize);
                for (size_t j = 0; j < resultSize; ++j) {
                    result[j] = (value1[j] * (1 - t) + value2[j] * t);
                }
                return result;
            } else if (interpolation == EDriverInterpolationType::SLERP) {
                assert(resultSize == 4);
                vkm::quat q1 = vkm::quat(value1[0], value1[1], value1[2], value1[3]);
                vkm::quat q2 = vkm::quat(value2[0], value2[1], value2[2], value2[3]);
                vkm::quat result = vkm::slerp(q1, q2, t);
                return std::vector<float> { result.x, result.y, result.z, result.w };
            }
            break;
        }
    }

    return std::nullopt;
}