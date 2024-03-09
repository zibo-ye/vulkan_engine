/* Copyright (c) 2018-2023, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

// Generates an irradiance cube from an environment map using convolution

#version 450

layout (location = 0) in vec3 inPos;
layout (location = 0) out vec4 outColor;
layout (binding = 0) uniform samplerCube samplerEnv;

layout(push_constant) uniform PushConsts {
	layout (offset = 64) float deltaPhi;
	layout (offset = 68) float deltaTheta;
} consts;

#define PI 3.1415926535897932384626433832795


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

void main()
{
	vec3 N = normalize(inPos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;

	vec3 color = vec3(0.0);
	uint sampleCount = 0u;
	for (float phi = 0.0; phi < TWO_PI; phi += consts.deltaPhi) {
		for (float theta = 0.0; theta < HALF_PI; theta += consts.deltaTheta) {
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
			color += rgbe_to_float(texture(samplerEnv, sampleVector)) * cos(theta) * sin(theta);
			sampleCount++;
		}
	}
	outColor = float_to_rgbe(PI * color / float(sampleCount));
}
