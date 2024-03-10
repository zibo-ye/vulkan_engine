//Based on https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/genbrdflut.frag

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // Ensure all necessary GLM components are included
#include <glm/gtx/norm.hpp> // For glm::length2 function if needed
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <vector>
#include <cmath>

const unsigned int NUM_SAMPLES = 1024u;
const float PI = 3.1415926536f;

float random(glm::vec2 co) {
    float a = 12.9898f;
    float b = 78.233f;
    float c = 43758.5453f;
    float dt= glm::dot(co, glm::vec2(a,b));
    float sn= fmod(dt, PI);
    return glm::fract(sin(sn) * c);
}

glm::vec2 hammersley2d(unsigned int i, unsigned int N) {
    unsigned int bits = (i << 16u) | (i >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    float rdi = float(bits) * 2.3283064365386963e-10f;
    return glm::vec2(float(i) / float(N), rdi);
}

// Based on http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_slides.pdf
glm::vec3 importanceSample_GGX(glm::vec2 Xi, float roughness, glm::vec3 normal) 
{
	// Maps a 2D point to a hemisphere with spread based on roughness
	float alpha = roughness * roughness;
	float phi = 2.0 * PI * Xi.x + random(glm::vec2(normal.x, normal.z)) * 0.1;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (alpha*alpha - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	glm::vec3 H = glm::vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

	// Tangent space
	glm::vec3 up = abs(normal.z) < 0.999 ? glm::vec3(0.0, 0.0, 1.0) : glm::vec3(1.0, 0.0, 0.0);
	glm::vec3 tangentX = normalize(cross(up, normal));
	glm::vec3 tangentY = normalize(cross(normal, tangentX));

	// Convert to world Space
	return normalize(tangentX * H.x + tangentY * H.y + normal * H.z);
}

// Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float k = (roughness * roughness) / 2.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

glm::vec2 BRDF(float NoV, float roughness)
{
	// Normal always points along z-axis for the 2D lookup 
	const glm::vec3 N = glm::vec3(0.0, 0.0, 1.0);
	glm::vec3 V = glm::vec3(sqrt(1.0 - NoV*NoV), 0.0, NoV);

	glm::vec2 LUT = glm::vec2(0.0);
	for(unsigned int i = 0u; i < NUM_SAMPLES; i++) {
		glm::vec2 Xi = hammersley2d(i, NUM_SAMPLES);
		glm::vec3 H = importanceSample_GGX(Xi, roughness, N);
		glm::vec3 L = 2.0f * glm::dot(V, H) * H - V;

		float dotNL = std::max(glm::dot(N, L), 0.0f);
		float dotNV = std::max(glm::dot(N, V), 0.0f);
		float dotVH = std::max(glm::dot(V, H), 0.0f); 
		float dotNH = std::max(glm::dot(H, N), 0.0f);

		if (dotNL > 0.0) {
			float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
			float G_Vis = (G * dotVH) / (dotNH * dotNV);
			float Fc = pow(1.0 - dotVH, 5.0);
			LUT += glm::vec2((1.0 - Fc) * G_Vis, Fc * G_Vis);
		}
	}
	return LUT / float(NUM_SAMPLES);
}

int main() {
    int Width = 512;
    int Height = 512;

    std::vector<glm::vec4> outputImage; // Assuming a 2D image of vec4's
    outputImage.resize(Width * Height);

    for (int y = 0; y < Height; ++y) {
        for (int x = 0; x < Width; ++x) {
            glm::vec2 inUV = glm::vec2(float(x) / Width, float(y) / Height);
            glm::vec4 outColor = glm::vec4(BRDF(inUV.s, 1.0f - inUV.t), 0.0f, 1.0f);
            outputImage[y * Width + x] = outColor;
        }
    }

    // Prepare the image data for stb_image_write
    std::vector<unsigned char> imageData(Width * Height * 4); // 4 components (RGBA) per pixel
    for (size_t i = 0; i < outputImage.size(); ++i) {
        glm::vec4& color = outputImage[i];
        imageData[i * 4 + 0] = static_cast<unsigned char>(color.r * 255.0f);
        imageData[i * 4 + 1] = static_cast<unsigned char>(color.g * 255.0f);
        imageData[i * 4 + 2] = static_cast<unsigned char>(color.b * 255.0f);
        imageData[i * 4 + 3] = static_cast<unsigned char>(color.a * 255.0f); // Assuming alpha is always 1.0
    }

    // Save the image as a PNG
    stbi_write_png("brdflut.png", Width, Height, 4, imageData.data(), Width * 4);

    return 0;
}
