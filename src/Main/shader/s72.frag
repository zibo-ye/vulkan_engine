#version 450
layout(location = 0) in highp vec3 fragPosition; 
layout(location = 1) in highp vec3 fragNormal;
layout(location = 2) in mediump vec4 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    highp vec3 n = normalize(fragNormal);
    highp vec3 light = mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), dot(n, vec3(0.0, 0.0, 1.0)) * 0.5 + 0.5);
    outColor = vec4(fragColor.rgb * light, fragColor.a); 
}
