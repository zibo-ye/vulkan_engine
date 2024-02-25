#include "Mesh.hpp"
#include "Material.hpp"

Mesh::Mesh(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::MESH)
{
    name = jsonObj["name"].getString();
    topology = GetVkPrimitiveTopology(jsonObj["topology"].getString());
    count = jsonObj["count"].getInt();

    if (jsonObj.hasKey("indices")) {
        indiceDescription = MeshIndices(jsonObj["indices"]);
    }

    auto& attributes = jsonObj["attributes"];
    for (auto& [attrName, attrVal] : attributes.getObject()) {
        this->attributeDescriptions[attrName] = MeshAttributes(attrVal, pScene.lock()->src);
    }

    if (jsonObj.hasKey("material")) {
        materialIdx = jsonObj["material"].getInt();
    }

    LoadMeshData();
}

void Mesh::LoadMeshData()
{
    meshData = std::make_shared<MeshData<NewVertex, uint32_t>>();

    // Load indices
    if (indiceDescription.has_value()) {
        std::ifstream ifs(indiceDescription->src, std::ios::binary);
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

        std::ifstream ifs(attrVal.src, std::ios::binary);
        if (!ifs) {
            throw std::runtime_error("Could not open file for reading: " + attrVal.src);
        }
        for (int i = 0; i < count; i++) {
            ifs.seekg(attrVal.offset + i * attrVal.stride);
            ifs.read(reinterpret_cast<char*>(&meshData->vertices[i]) + memOffset, GetVkFormatByteSize(attrVal.format));
        }
        if (!ifs) {
            throw std::runtime_error("Error Reading file: " + attrVal.src);
        }
    }


    // vertices -> indexed vertices
    std::unordered_map<NewVertex, uint32_t> uniqueVertices;
    std::vector<NewVertex> uniqueVertexList;
    meshData->indices = std::vector<uint32_t>();
    meshData->indices->reserve(count);

    for (const auto& vertex : meshData->vertices) {
        if (uniqueVertices.count(vertex) == 0) {
            uniqueVertices[vertex] = static_cast<uint32_t>(uniqueVertexList.size());
            uniqueVertexList.push_back(vertex);
        }
        meshData->indices->push_back(uniqueVertices[vertex]);
    }

    meshData->vertices = std::move(uniqueVertexList);

    // Update bounds
    for (auto& vertex : meshData->vertices) {
        UpdateBounds(vertex);
    }
}

void Mesh::UpdateBounds(const NewVertex& vertex)
{
    if (vertex.position.x() < min.x())
        min.x() = vertex.position.x();
    if (vertex.position.y() < min.y())
        min.y() = vertex.position.y();
    if (vertex.position.z() < min.z())
        min.z() = vertex.position.z();
    if (vertex.position.x() > max.x())
        max.x() = vertex.position.x();
    if (vertex.position.y() > max.y())
        max.y() = vertex.position.y();
    if (vertex.position.z() > max.z())
        max.z() = vertex.position.z();
}

bool Mesh::isUsingSimpleMaterial() const
{
    return !materialIdx.has_value() || pScene.lock()->materials[*materialIdx]->isSimple;
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
