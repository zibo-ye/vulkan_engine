#include "Mesh.hpp"

Mesh::Mesh(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::MESH)
{
    name = jsonObj["name"].getString();
    topology = GetVkPrimitiveTopology(jsonObj["topology"].getString());
    count = jsonObj["count"].getInt();

    if (jsonObj.getObject().find("indices") != jsonObj.getObject().end()) {
        indiceDescription = MeshIndices(jsonObj["indices"]);
    }

    auto& attributes = jsonObj["attributes"];
    for (auto& [attrName, attrVal] : attributes.getObject()) {
        this->attributeDescriptions[attrName] = MeshAttributes(attrVal, pScene.lock()->src);
    }

    LoadMeshData();
}

void Mesh::LoadMeshData()
{
    meshData = std::make_shared<MeshData<NewVertex, uint32_t>>();

    // Load indices
    if (indiceDescription.has_value()) {
        std::ifstream ifs(indiceDescription->src);
        if (!ifs) {
            throw std::runtime_error("Could not open file for reading: " + indiceDescription->src);
        }

        // offset -> offset + count * sizeof(index)
        ifs.seekg(indiceDescription->offset);
        meshData->indices = std::vector<uint32_t>(count);
        ifs.read(reinterpret_cast<char*>(meshData->indices.value().data()), count * GetVkFormatByteSize(indiceDescription->format));
    }

    meshData->vertices = std::vector<NewVertex>(count);

    for (auto& [attrName, attrVal] : attributeDescriptions) {
        int memOffset = NewVertex::getAttributeOffset(attrName);

        std::ifstream ifs(attrVal.src);
        if (!ifs) {
            throw std::runtime_error("Could not open file for reading: " + attrVal.src);
        }
        for (int i = 0; i < count; i++) {
            ifs.seekg(attrVal.offset + i * attrVal.stride);
            ifs.read(reinterpret_cast<char*>(&meshData->vertices[i]) + memOffset, GetVkFormatByteSize(attrVal.format));
        }
    }
}

MeshIndices::MeshIndices(const Utility::json::JsonValue& jsonObj)
{
    src = jsonObj["src"].getString();
    offset = jsonObj["offset"].getInt();
    format = GetVkFormat(jsonObj["format"].getString());
}

MeshAttributes::MeshAttributes(const Utility::json::JsonValue& jsonObj, const std::string& scenePath)
{
    std::filesystem::path scenePathFS = scenePath;
    src = (scenePathFS.parent_path() / jsonObj["src"].getString()).string();
    offset = jsonObj["offset"].getInt();
    stride = jsonObj["stride"].getInt();
    format = GetVkFormat(jsonObj["format"].getString());
}
