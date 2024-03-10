#include "Material.hpp"
#include "Graphics/Vulkan/VulkanInitializer.hpp"
#include "SceneEnum.hpp"
#include "Texture.hpp"

Material::Material(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::MATERIAL)
{
    name = jsonObj["name"].getString();

	if (jsonObj.hasKey("normalMap")) {
		normalMap = Texture(jsonObj["normalMap"], pScene.lock()->src, VK_FORMAT_R8G8B8A8_UNORM);
	}

    if (jsonObj.hasKey("displacementMap")) {
        displacementMap = Texture(jsonObj["displacementMap"], pScene.lock()->src, VK_FORMAT_R8G8B8A8_UNORM);
    }

    if (jsonObj.hasKey("pbr")) {
        type = EMaterialType::PBR;
        pbr = PBR();

        if (jsonObj["pbr"].hasKey("albedo")) {
            auto albedo = jsonObj["pbr"]["albedo"];
            if (albedo.isArray()) {
                auto vec = albedo.getVecFloat();
                pbr->albedoMap = Texture(vkm::vec3(vec[0], vec[1], vec[2]), VK_FORMAT_R8G8B8A8_SRGB);
            } else {
                pbr->albedoMap = Texture(albedo, pScene.lock()->src, VK_FORMAT_R8G8B8A8_SRGB);
            }
        }

        if (jsonObj["pbr"].hasKey("roughness")) {
            auto roughness = jsonObj["pbr"]["roughness"];
            if (roughness.isFloat()) {
                pbr->roughnessMap = Texture(roughness.getFloat(), VK_FORMAT_R8_UNORM);
            } else {
                pbr->roughnessMap = Texture(roughness, pScene.lock()->src, VK_FORMAT_R8_UNORM);
            }
        }

        if (jsonObj["pbr"].hasKey("metalness")) {
            auto metalness = jsonObj["pbr"]["metalness"];
            if (metalness.isFloat()) {
                pbr->metalnessMap = Texture(metalness.getFloat(), VK_FORMAT_R8_UNORM);
            } else {
                pbr->metalnessMap = Texture(metalness, pScene.lock()->src, VK_FORMAT_R8_UNORM);
            }
        }
    } else if (jsonObj.hasKey("lambertian")) {
        type = EMaterialType::LAMBERTIAN;
        lambertian = Lambertian();

        if (jsonObj["lambertian"].hasKey("albedo")) {
            auto albedo = jsonObj["lambertian"]["albedo"];
            if (albedo.isArray()) {
                auto vec = albedo.getVecFloat();
                lambertian->albedoMap = Texture(vkm::vec3(vec[0], vec[1], vec[2]), VK_FORMAT_R8G8B8A8_SRGB);
            } else {
                lambertian->albedoMap = Texture(albedo, pScene.lock()->src, VK_FORMAT_R8G8B8A8_SRGB);
            }
        }
    } else if (jsonObj.hasKey("mirror")) {
        type = EMaterialType::MIRROR;
    } else if (jsonObj.hasKey("environment")) {
        type = EMaterialType::ENVIRONMENT;
    } else if (jsonObj.hasKey("simple")) {
        type = EMaterialType::SIMPLE;
    }
}

void Material::InitDescriptorSet(VulkanCore* pVulkanCore)
{
    if (descriptorSet != VK_NULL_HANDLE)
        return;
    createDescriptorSet(pVulkanCore->GetDevice(), pVulkanCore->descriptorSetLayouts[1], pVulkanCore->descriptorPool, descriptorSet);

    if (pbr) {
        pbr->albedoMap.uploadTextureToGPU(pVulkanCore);
        VkDescriptorImageInfo imageInfo = pbr->albedoMap.textureImage.GetDescriptorImageInfo();
        VkWriteDescriptorSet descriptorWrite {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);

        pbr->roughnessMap.uploadTextureToGPU(pVulkanCore);
        imageInfo = pbr->roughnessMap.textureImage.GetDescriptorImageInfo();
        descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);

        pbr->metalnessMap.uploadTextureToGPU(pVulkanCore);
        imageInfo = pbr->metalnessMap.textureImage.GetDescriptorImageInfo();
        descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);

    } else if (lambertian) {
        lambertian->albedoMap.uploadTextureToGPU(pVulkanCore);
        VkDescriptorImageInfo imageInfo = lambertian->albedoMap.textureImage.GetDescriptorImageInfo();
        VkWriteDescriptorSet descriptorWrite {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);

        imageInfo = g_emptyTexture->textureImage.GetDescriptorImageInfo();
        descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);
        descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    } else {
        VkDescriptorImageInfo imageInfo = g_emptyTexture->textureImage.GetDescriptorImageInfo();
        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);
        descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);
        descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageInfo,
        };
        vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }
    normalMap.uploadTextureToGPU(pVulkanCore);
    VkDescriptorImageInfo imageInfo = normalMap.textureImage.GetDescriptorImageInfo();
    VkWriteDescriptorSet descriptorWrite {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = 3,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfo,
    };
    vkUpdateDescriptorSets(pVulkanCore->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}

std::shared_ptr<Material> g_SimpleMaterial = std::make_shared<Material>();