#include "math.hpp"
#include <cassert>

// RH, 0-1, Y-down, Z-in
vkm::mat4 vkm::perspective(float fovY, float aspect, float zNear, float zFar)
{
    assert(abs(aspect - std::numeric_limits<float>::epsilon()) > 0);
    mat4 result(0.0f); // Initialize to zero
    const float tanHalfFovy = tan(fovY / 2.0f);

    result[0][0] = 1.0f / (aspect * tanHalfFovy);
    result[1][1] = -1.0f / (tanHalfFovy);
    result[2][2] = zFar / (zNear - zFar);
    result[3][2] = -(zFar * zNear) / (zFar - zNear);
    result[2][3] = -1.0f;
    return result;
}

vkm::mat4 vkm::lookAt(const vec3& eye, const vec3& center, const vec3& up)
{
    vec3 f = center - eye;
    f = normalize(f);
    vec3 s = normalize(cross(f, up));
    vec3 u = cross(s, f);

    mat4 result;
    result[0][0] = s.x();
    result[1][0] = s.y();
    result[2][0] = s.z();
    result[0][1] = u.x();
    result[1][1] = u.y();
    result[2][1] = u.z();
    result[0][2] = -f.x();
    result[1][2] = -f.y();
    result[2][2] = -f.z();
    result[3][0] = -dot(s, eye);
    result[3][1] = -dot(u, eye);
    result[3][2] = dot(f, eye);
    return result;
}

vkm::mat4 vkm::rotate(const mat4& m, float angle, const vec3& axis)
{
    float c = cos(angle);
    float s = sin(angle);
    vec3 normAxis = normalize(axis);

    vec3 temp = (1.0f - c) * normAxis;

    mat4 Rotate;
    Rotate(0, 0) = c + temp[0] * normAxis[0];
    Rotate(0, 1) = 0 + temp[0] * normAxis[1] + s * normAxis[2];
    Rotate(0, 2) = 0 + temp[0] * normAxis[2] - s * normAxis[1];

    Rotate(1, 0) = 0 + temp[1] * normAxis[0] - s * normAxis[2];
    Rotate(1, 1) = c + temp[1] * normAxis[1];
    Rotate(1, 2) = 0 + temp[1] * normAxis[2] + s * normAxis[0];

    Rotate(2, 0) = 0 + temp[2] * normAxis[0] + s * normAxis[1];
    Rotate(2, 1) = 0 + temp[2] * normAxis[1] - s * normAxis[0];
    Rotate(2, 2) = c + temp[2] * normAxis[2];

    mat4 Result;
    Result[0] = m[0] * Rotate(0, 0) + m[1] * Rotate(0, 1) + m[2] * Rotate(0, 2);
    Result[1] = m[0] * Rotate(1, 0) + m[1] * Rotate(1, 1) + m[2] * Rotate(1, 2);
    Result[2] = m[0] * Rotate(2, 0) + m[1] * Rotate(2, 1) + m[2] * Rotate(2, 2);
    Result[3] = m[3];
    return Result;
}

vkm::mat4 vkm::scale(const mat4& m, const vec3& v)
{
    mat4 Result(m);
    Result[0] *= v[0];
    Result[1] *= v[1];
    Result[2] *= v[2];
    return Result;
}

vkm::mat4 vkm::translate(const vec3& v)
{
    mat4 result; // Start with the original matrix

    // Apply translation
    for (std::size_t i = 0; i < 3; ++i) {
        result[3][i] = v[i];
    }
    return result;
}
