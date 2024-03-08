#version 450

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    mat4 viewproj; 
    vec4 position;
} ubo_cam;

layout (set = 0, binding = 1) uniform samplerCube cubeMapTexture;

layout (set = 1, binding = 0) uniform sampler2D ALBEDO;
layout (set = 1, binding = 1) uniform sampler2D ROUGHNESS;
layout (set = 1, binding = 2) uniform sampler2D METALNESS;
layout (set = 1, binding = 3) uniform sampler2D NORMAL;
layout (set = 1, binding = 4) uniform sampler2D DISPLACEMENT;

// uniform samplerCube LAMBERTIAN;
// uniform samplerCube GGX;
// uniform samplerCube RADIANCE; //redundant with ggx but useful for debugging


struct FragData {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec2 texCoord;
    vec3 tangent;
    vec3 bitangent;
};
layout(location = 0) in FragData fragData;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 matWorld;
    mat4 matNormal; //transpose(inv(matWorld))
} pushConstants;

// layout(constant_id = 0) const int materialType = 4; // use a specialized constant to pass the material type so the uber shader won't be too big.

vec4 PBRMaterial()
{
    return texture(cubeMapTexture, fragData.normal);
}

vec4 LambertianMaterial()
{
    return texture(cubeMapTexture, fragData.normal);
}

vec4 MirrorMaterial()
{
    vec3 inDir = normalize(fragData.position - ubo_cam.position.xyz);
    vec3 reflectDir = reflect(inDir, normalize(fragData.normal));
    return texture(cubeMapTexture, reflectDir);
}

vec4 SimpleMaterial()
{
    vec3 n = normalize(fragData.normal);
    vec3 light = mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), dot(n, vec3(0.0, 0.0, 1.0)) * 0.5 + 0.5);
    return vec4(fragData.color.rgb * light, fragData.color.a); 
}

vec4 EnvironmentMaterial()
{
    return texture(cubeMapTexture, fragData.normal);
}

// input and output are all in srgb linear space
// Based on http://www.oscars.org/science-technology/sci-tech-projects/aces
// Aces tonemapping: from linear srgb to aces to linear srgb
// #TODO: a better way is to tonemap the final framebuffer, not the model color (could be transparent, etc.) 
vec3 aces_tonemap(vec3 color){	
	// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
	mat3 m1 = mat3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
	);

	color = m1 * color;    
    // Apply RRT and ODT
    vec3 a = color * (color + 0.0245786) - 0.000090537;
    vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
    color  = a / b;

	// ODT_SAT => XYZ => D60_2_D65 => sRGB
	mat3 m2 = mat3(
        1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
	);

    color = m2 * color;
    // Clamp to [0, 1]
	return clamp(color, 0.0, 1.0);	
}

void main() {
    int materialType = int(pushConstants.matNormal[3][3]);
    switch(materialType) {
        case 0: // PBR
            outColor = PBRMaterial();
            break;
        case 1: // LAMBERTIAN
            outColor = LambertianMaterial();
            break;
        case 2: // MIRROR
            outColor = MirrorMaterial();
            break;
        case 3: // ENVIRONMENT
            outColor = EnvironmentMaterial();
            break;
        case 4: // SIMPLE
            outColor = SimpleMaterial();
            break;
        default:
            outColor = SimpleMaterial(); // Default case
    }
    outColor = vec4(aces_tonemap(outColor.rgb), outColor.a);
}