#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPosition; 
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragColor;
layout(location = 3) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    // vec3 n = normalize(fragNormal);
    // vec3 light = mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), dot(n, vec3(0.0, 0.0, 1.0)) * 0.5 + 0.5);
    // outColor = vec4(fragColor.rgb * light, fragColor.a); 

    outColor = texture(texSampler, fragTexCoord);
}
