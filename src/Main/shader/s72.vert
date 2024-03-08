#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

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
    gl_Position = ubo.proj * ubo.view * pushConstants.matWorld * vec4(inPosition,  1.0);
}
