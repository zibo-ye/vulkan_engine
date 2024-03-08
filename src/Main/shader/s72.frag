#version 450

layout(binding = 1) uniform samplerCube cubeMapTexture;

struct FragData {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec2 texCoord;
};
layout(location = 0) in FragData fragData;

layout(location = 0) out vec4 outColor;

void main() {
    // vec3 n = normalize(fragNormal);
    // vec3 light = mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), dot(n, vec3(0.0, 0.0, 1.0)) * 0.5 + 0.5);
    // outColor = vec4(fragColor.rgb * light, fragColor.a); 
    outColor = texture(cubeMapTexture, fragData.normal);
}
