#include "Mesh.hpp"

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
        this->attributes[attrName] = MeshAttributes(attrVal, pScene.lock()->src);
    }
}

//VkPipelineVertexInputStateCreateInfo Mesh::getVertexInputInfo() const
//{
//    // TODO: Dynamic vertex input
//
//    //    auto bindingDescription = Vertex::getBindingDescription();
//    // auto attributeDescriptions = Vertex::getAttributeDescriptions();
//
//    // VkPipelineVertexInputStateCreateInfo vertexInputInfo {
//    //     .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
//    //     .vertexBindingDescriptionCount = 1,
//    //     .pVertexBindingDescriptions = &bindingDescription,
//    //     .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
//    //     .pVertexAttributeDescriptions = attributeDescriptions.data(),
//    // };
//
//    // return vertexInputInfo;
//}

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
//
//void MeshAttributes::LoadData()
//{
//    std::ifstream ifs(src);
//    if (!ifs) {
//        throw std::runtime_error("Could not open file for reading: " + src);
//    }
//    //get file size
//    ifs.seekg(0, std::ios::end);
//    size_t size = ifs.tellg();
//    ifs.seekg(0, std::ios::beg);
//
//    int count = size / stride;
//    data = std::vector<char>(GetVkFormatByteSize(format) * count);
//    
//    for (int i = 0; i < count; i++) {
//		ifs.seekg(offset + i * stride);
//		ifs.read(data->data() + i * GetVkFormatByteSize(format), GetVkFormatByteSize(format));
//	}
//}

//VkVertexInputAttributeDescription MeshAttributes::makeVertexInputAttributeDescription(size_t location) const
//{
//	return {
//		.location = static_cast<uint32_t>(location),
//		.binding = 0,
//		.format = format,
//		.offset = static_cast<uint32_t>(offset), //This is wrong at the moment, the offset variable is in file, not the memory layout.
//	};
//}
