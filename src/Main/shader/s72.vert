#version 450


layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    mat4 viewproj; 
    vec4 position;
} ubo_cam;

layout (set = 1, binding = 3) uniform sampler2D NORMAL;

layout(location = 0) in vec3 inPosition; 
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec4 inColor;

struct FragData {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec2 texCoord;
    vec3 tangent;
    vec3 bitangent;
};
layout(location = 0) out FragData fragData;

layout(push_constant) uniform PushConstants {
    mat4 matWorld;
    mat4 matNormal; //transpose(inv(matWorld))
} pushConstants;


void main() {
    fragData.position = vec3(pushConstants.matWorld * vec4(inPosition, 1.0)); // Transform position by light matrix
    fragData.normal = mat3(pushConstants.matNormal) * inNormal; 
    fragData.color = inColor; // Pass color directly
    fragData.texCoord = inTexCoord;
    fragData.tangent = mat3(pushConstants.matWorld) * inTangent.rgb;
    fragData.bitangent = inTangent.w * cross(fragData.normal, fragData.tangent);

    gl_Position = ubo_cam.proj * ubo_cam.view * pushConstants.matWorld * vec4(inPosition,  1.0);
}