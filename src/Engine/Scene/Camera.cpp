
#include "Camera.hpp"
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
