#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 lightFromLocal;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition; 
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec4 fragColor;

layout( push_constant ) uniform constants
{
	mat4 model;
} PushConstants;

void main() {
    fragPosition = vec3(ubo.lightFromLocal * vec4(inPosition, 1.0)); // Transform position by light matrix
    fragNormal = mat3(ubo.lightFromLocal) * inNormal; // Transform normal by light matrix (3x3 part)
    fragColor = inColor; // Pass color directly
    gl_Position = ubo.proj * ubo.view * PushConstants.model * vec4(inPosition,  1.0);
}
