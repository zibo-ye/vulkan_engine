#version 450

layout(set = 0, binding = 0) uniform CameraData {
    mat4 view;
    mat4 proj;
    mat4 viewproj; 
    vec4 position;
} ubo_cam;

layout (set = 0, binding = 1) uniform samplerCube ENV_RADIANCE;
layout (set = 0, binding = 2) uniform samplerCube LAMBERTIAN;
layout (set = 0, binding = 3) uniform samplerCube irradiance;
layout (set = 0, binding = 4) uniform samplerCube prefilteredMap;
layout (set = 0, binding = 5) uniform sampler2D samplerBRDFLUT;

layout (set = 1, binding = 0) uniform sampler2D ALBEDO;
layout (set = 1, binding = 1) uniform sampler2D ROUGHNESS;
layout (set = 1, binding = 2) uniform sampler2D METALNESS;
layout (set = 1, binding = 3) uniform sampler2D NORMAL;
// layout (set = 1, binding = 4) uniform sampler2D DISPLACEMENT;

// uniform samplerCube GGX;


struct FragData {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec2 texCoord;
    vec3 tangent;
    vec3 bitangent;
};
layout(location = 0) in FragData inFragData;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants {
    mat4 matWorld;
    mat4 matNormal; //transpose(inv(matWorld))
} pushConstants;

// layout(constant_id = 0) const int materialType = 4; // use a specialized constant to pass the material type so the uber shader won't be too big.

vec3 rgbe_to_float(vec4 rgbe) {
    if (rgbe == vec4(0.0, 0.0, 0.0, 0.0)) {
        return vec3(0.0);
    }
    int exp = int(rgbe.a * 255.0) - 128;
    return vec3(
        ldexp((rgbe.r * 255.0 + 0.5) / 256.0, exp),
        ldexp((rgbe.g * 255.0 + 0.5) / 256.0, exp),
        ldexp((rgbe.b * 255.0 + 0.5) / 256.0, exp)
    );
}

vec4 float_to_rgbe(vec3 color) {
    float d = max(color.r, max(color.g, color.b));

    if (d <= 1e-32) {
        return vec4(0, 0, 0, 0); // Early return for very small d
    }

    int e;
    float m = frexp(d, e); // Extract mantissa and exponent
    float fac = 255.999 * (m / d);

    if (e > 127) {
        return vec4(0xff, 0xff, 0xff, 0xff); // Clamp to bright white for large e
    }

    // Scale and store
    return vec4(
        max(0, int(color.r * fac))/255.0,
        max(0, int(color.g * fac))/255.0,
        max(0, int(color.b * fac))/255.0,
        (e + 128)/255.0 // Add bias to exponent
    );
}


// PBR Material is based on the implementation: https://github.com/SaschaWillems/Vulkan-glTF-PBR
const float M_PI = 3.141592653589793;
const float c_MinRoughness = 0.04;

struct PBRInfo
{
	float NdotL;                  // cos angle between normal and light direction
	float NdotV;                  // cos angle between normal and view direction
	float NdotH;                  // cos angle between normal and half vector
	float LdotH;                  // cos angle between light direction and half vector
	float VdotH;                  // cos angle between view direction and half vector
	float perceptualRoughness;    // roughness value, as authored by the model creator (input to shader)
	float metalness;              // metallic value at the surface
	vec3 reflectance0;            // full reflectance color (normal incidence angle)
	vec3 reflectance90;           // reflectance color at grazing angle
	float alphaRoughness;         // roughness mapped to a more linear change in the roughness (proposed by [2])
	vec3 diffuseColor;            // color contribution from diffuse lighting
	vec3 specularColor;           // color contribution from specular lighting
};

// Calculation of the lighting contribution from an optional Image Based Light source.
// Precomputed Environment Maps are required uniform inputs and are computed as outlined in [1].
// See our README.md on Environment Maps [3] for additional discussion.
vec3 getIBLContribution(PBRInfo pbrInputs, vec3 n, vec3 reflection)
{
	// float lod = (pbrInputs.perceptualRoughness * prefilteredCubeMipLevels); //TODO: prefilteredCubeMipLevels
	float lod = (pbrInputs.perceptualRoughness * 1);
	// retrieve a scale and bias to F0. See [1], Figure 3
	vec3 brdf = (texture(samplerBRDFLUT, vec2(pbrInputs.NdotV, 1.0 - pbrInputs.perceptualRoughness))).rgb;
	vec3 diffuseLight = rgbe_to_float(texture(irradiance, n));

	vec3 specularLight = rgbe_to_float(textureLod(prefilteredMap, reflection, lod)).rgb;

	vec3 diffuse = diffuseLight * pbrInputs.diffuseColor;
	vec3 specular = specularLight * (pbrInputs.specularColor * brdf.x + brdf.y); //TODO: brdf

	return diffuse + specular;
}

// Basic Lambertian diffuse
// Implementation from Lambert's Photometria https://archive.org/details/lambertsphotome00lambgoog
// See also [1], Equation 1
vec3 diffuse(PBRInfo pbrInputs)
{
	return pbrInputs.diffuseColor / M_PI;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
vec3 specularReflection(PBRInfo pbrInputs)
{
	return pbrInputs.reflectance0 + (pbrInputs.reflectance90 - pbrInputs.reflectance0) * pow(clamp(1.0 - pbrInputs.VdotH, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()),
// where rougher material will reflect less light back to the viewer.
// This implementation is based on [1] Equation 4, and we adopt their modifications to
// alphaRoughness as input as originally proposed in [2].
float geometricOcclusion(PBRInfo pbrInputs)
{
	float NdotL = pbrInputs.NdotL;
	float NdotV = pbrInputs.NdotV;
	float r = pbrInputs.alphaRoughness;

	float attenuationL = 2.0 * NdotL / (NdotL + sqrt(r * r + (1.0 - r * r) * (NdotL * NdotL)));
	float attenuationV = 2.0 * NdotV / (NdotV + sqrt(r * r + (1.0 - r * r) * (NdotV * NdotV)));
	return attenuationL * attenuationV;
}

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float microfacetDistribution(PBRInfo pbrInputs)
{
	float roughnessSq = pbrInputs.alphaRoughness * pbrInputs.alphaRoughness;
	float f = (pbrInputs.NdotH * roughnessSq - pbrInputs.NdotH) * pbrInputs.NdotH + 1.0;
	return roughnessSq / (M_PI * f * f);
}

// Gets metallic factor from specular glossiness workflow inputs 
float convertMetallic(vec3 diffuse, vec3 specular, float maxSpecular) {
	float perceivedDiffuse = sqrt(0.299 * diffuse.r * diffuse.r + 0.587 * diffuse.g * diffuse.g + 0.114 * diffuse.b * diffuse.b);
	float perceivedSpecular = sqrt(0.299 * specular.r * specular.r + 0.587 * specular.g * specular.g + 0.114 * specular.b * specular.b);
	if (perceivedSpecular < c_MinRoughness) {
		return 0.0;
	}
	float a = c_MinRoughness;
	float b = perceivedDiffuse * (1.0 - maxSpecular) / (1.0 - c_MinRoughness) + perceivedSpecular - 2.0 * c_MinRoughness;
	float c = c_MinRoughness - perceivedSpecular;
	float D = max(b * b - 4.0 * a * c, 0.0);
	return clamp((-b + sqrt(D)) / (2.0 * a), 0.0, 1.0);
}

vec4 PBRMaterial(FragData fragData)
{
    // vec4 result = texture(ALBEDO, fragData.texCoord);
    // return result;

	float perceptualRoughness = texture(ROUGHNESS, fragData.texCoord).r;
	float metallic = texture(METALNESS, fragData.texCoord).r;
	vec4 baseColor = texture(ALBEDO, fragData.texCoord); // MAYBE need to convert to linear
	vec3 diffuseColor;

	vec3 f0 = vec3(0.04);

	baseColor *= fragData.color;
	diffuseColor = baseColor.rgb * (vec3(1.0) - f0);
	diffuseColor *= 1.0 - metallic;
		
	float alphaRoughness = perceptualRoughness * perceptualRoughness;

	vec3 specularColor = mix(f0, baseColor.rgb, metallic);

	// Compute reflectance.
	float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

	// For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specularEnvironmentR0 = specularColor.rgb;
	vec3 specularEnvironmentR90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	vec3 n = fragData.normal;
	vec3 v = normalize(ubo_cam.position.rgb - fragData.position);    // Vector from surface point to camera
	vec3 l = vec3(0,0,1);     // Vector from surface point to light
	// vec3 l = normalize(uboParams.lightDir.xyz);     // Vector from surface point to light
	vec3 h = normalize(l+v);                        // Half vector between both l and v
	vec3 reflection = -normalize(reflect(v, n));
	reflection.y *= -1.0f;

	float NdotL = clamp(dot(n, l), 0.001, 1.0);
	float NdotV = clamp(abs(dot(n, v)), 0.001, 1.0);
	float NdotH = clamp(dot(n, h), 0.0, 1.0);
	float LdotH = clamp(dot(l, h), 0.0, 1.0);
	float VdotH = clamp(dot(v, h), 0.0, 1.0);

	PBRInfo pbrInputs = PBRInfo(
		NdotL,
		NdotV,
		NdotH,
		LdotH,
		VdotH,
		perceptualRoughness,
		metallic,
		specularEnvironmentR0,
		specularEnvironmentR90,
		alphaRoughness,
		diffuseColor,
		specularColor
	);

	// Calculate the shading terms for the microfacet specular shading model
	vec3 F = specularReflection(pbrInputs);
	float G = geometricOcclusion(pbrInputs);
	float D = microfacetDistribution(pbrInputs);

	const vec3 u_LightColor = vec3(1.0);

	// Calculation of analytical lighting contribution
	vec3 diffuseContrib = (1.0 - F) * diffuse(pbrInputs);
	vec3 specContrib = F * G * D / (4.0 * NdotL * NdotV);
	// Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
	vec3 color = NdotL * u_LightColor * (diffuseContrib + specContrib);

	// Calculate lighting contribution from image based lighting source (IBL)
	color += getIBLContribution(pbrInputs, n, reflection);
	
	return vec4(color, baseColor.a);
}

vec4 LambertianMaterial(FragData fragData)
{
    vec3 lambertian_sample_light = rgbe_to_float(texture(LAMBERTIAN, fragData.normal));
    vec4 albedo = texture(ALBEDO, fragData.texCoord);

    return vec4(albedo.rgb * lambertian_sample_light, albedo.a);
}

vec4 MirrorMaterial(FragData fragData)
{
    vec3 inDir = normalize(fragData.position - ubo_cam.position.xyz);
    vec3 reflectDir = reflect(inDir, normalize(fragData.normal));

    vec4 result = texture(ENV_RADIANCE, reflectDir); //RGBE
    return vec4(rgbe_to_float(result),1);
}

vec4 SimpleMaterial(FragData fragData)
{
    vec3 n = normalize(fragData.normal);
    vec3 light = mix(vec3(0.0, 0.0, 0.0), vec3(1.0, 1.0, 1.0), dot(n, vec3(0.0, 0.0, 1.0)) * 0.5 + 0.5);
    return vec4(fragData.color.rgb * light, fragData.color.a); 
}

vec4 EnvironmentMaterial(FragData fragData)
{
    vec4 result = texture(ENV_RADIANCE, fragData.normal); //RGBE
    return vec4(rgbe_to_float(result),1);
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

vec3 adjustNormal(vec3 normal, vec3 tangent, vec3 bitangent, vec3 normalMap) {
    mat3 tbn = mat3(tangent, bitangent, normal);
    return normalize(tbn * (normalMap * 2.0 - 1.0));
}

void main() {
    int materialType = int(pushConstants.matNormal[3][3]);
    FragData fragData = inFragData;
    fragData.normal = adjustNormal(fragData.normal, fragData.tangent, fragData.bitangent, texture(NORMAL, fragData.texCoord).xyz); 

    switch(materialType) {
        case 0: // PBR
            outColor = PBRMaterial(fragData);
            break;
        case 1: // LAMBERTIAN
            outColor = LambertianMaterial(fragData);
            break;
        case 2: // MIRROR
            outColor = MirrorMaterial(fragData);
            break;
        case 3: // ENVIRONMENT
            outColor = EnvironmentMaterial(fragData);
            break;
        case 4: // SIMPLE
            outColor = SimpleMaterial(fragData);
            break;
        default:
            outColor = SimpleMaterial(fragData); // Default case
    }
    outColor = vec4(aces_tonemap(outColor.rgb), outColor.a);
}