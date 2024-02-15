
#include "Camera.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"

void UserCamera::UpdateCameraParameters(UserCameraUpdateParameters params)
{
    if (params.fromPos) {
        fromPos = *params.fromPos;
    }
    if (params.lookAtPos) {
        lookAtPos = *params.lookAtPos;
    }
    if (params.aspect) {
        perspective.aspect = *params.aspect;
    }
    if (params.vfov) {
        perspective.vfov = *params.vfov;
    }
    if (params.near_plane) {
        perspective.near_plane = *params.near_plane;
    }
    if (params.far_plane) {
        perspective.far_plane = *params.far_plane;
    }
}

glm::mat4 SceneCamera::getViewMatrix() const
{
    glm::mat4 InvViewMatrix = glm::mat4(1.0f);

    size_t childIdx = index;
    while (true) {
        auto pSScene = pScene.lock();
        if (pSScene->nodeParents.find(childIdx) == pSScene->nodeParents.end()) {
            break;
        }
        auto parentIdxs = pSScene->nodeParents.at(childIdx);

        // for (auto parentIdx : parentIdxs) {
        auto parent = pSScene->nodes.at(parentIdxs[0]); // Assuming only one parent for now
        InvViewMatrix = parent->GetTransform() * InvViewMatrix;
        childIdx = parentIdxs[0];
    }

    return glm::inverse(InvViewMatrix);
}

bool ICamera::FrustumCulling(std::shared_ptr<Mesh> pMesh, glm::mat4& worldTransform)
{
    glm::mat4 viewMatrix = getViewMatrix();
    glm::mat4 projMatrix = getProjectionMatrix();

    glm::mat4 mvp = projMatrix * viewMatrix * worldTransform;

    // Define the 8 corners of the AABB
    std::vector<glm::vec3> corners = {
        { pMesh->min.x, pMesh->min.y, pMesh->min.z },
        { pMesh->max.x, pMesh->min.y, pMesh->min.z },
        { pMesh->min.x, pMesh->max.y, pMesh->min.z },
        { pMesh->max.x, pMesh->max.y, pMesh->min.z },
        { pMesh->min.x, pMesh->min.y, pMesh->max.z },
        { pMesh->max.x, pMesh->min.y, pMesh->max.z },
        { pMesh->min.x, pMesh->max.y, pMesh->max.z },
        { pMesh->max.x, pMesh->max.y, pMesh->max.z }
    };

    // Transform each corner to clip space and perform culling check
    bool allOutsideOnePlane = true;
    for (int i = 0; i < 6; ++i) { // For each frustum plane
        allOutsideOnePlane = true;

        for (const auto& corner : corners) {
            glm::vec4 transformedCorner = mvp * glm::vec4(corner, 1.0f);

            // Perspective division
            transformedCorner /= transformedCorner.w;

            switch (i) {
            case 0:
                allOutsideOnePlane &= (transformedCorner.x < -1.0f);
                break; // Left
            case 1:
                allOutsideOnePlane &= (transformedCorner.x > 1.0f);
                break; // Right
            case 2:
                allOutsideOnePlane &= (transformedCorner.y < -1.0f);
                break; // Bottom
            case 3:
                allOutsideOnePlane &= (transformedCorner.y > 1.0f);
                break; // Top
            case 4:
                allOutsideOnePlane &= (transformedCorner.z < -1.0f);
                break; // Near
            case 5:
                allOutsideOnePlane &= (transformedCorner.z > 1.0f);
                break; // Far
            }

            // If any corner is inside the plane, move to the next plane
            if (!allOutsideOnePlane)
                break;
        }

        // If all corners are outside one plane, the AABB is outside the frustum
        if (allOutsideOnePlane)
            return false;
    }

    // The AABB is potentially visible
    return true;
}