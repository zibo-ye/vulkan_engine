#include "math.hpp"
#include <cassert>

// RH, 0-1, Y-down, Z-in
vkm::mat4 vkm::perspective(float fovY, float aspect, float zNear, float zFar)
{
    assert(abs(aspect - std::numeric_limits<float>::epsilon()) > 0);
    mat4 result; // Initialize to zero
    const float tanHalfFovy = tan(fovY / 2.0f);

    result[0][0] = 1.0f / (aspect * tanHalfFovy);
    result[1][1] = -1.0f / (tanHalfFovy);
    result[2][2] = zFar / (zNear - zFar);
    result[3][2] = -(zFar * zNear) / (zFar - zNear);
    result[2][3] = -1.0f;
    result[3][3] = 0.0f;
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

vkm::mat4 vkm::translate(const mat4& m, const vec3& v)
{
    mat4 result(m); // Start with the original matrix

	// Apply translation
    for (std::size_t i = 0; i < 3; ++i) {
		result[3][i] += v[i];
	}
	return result;
}

vkm::mat3 vkm::mat3_cast(const quat& q)
{
    mat3 Result;
    float qxx(q.x * q.x);
    float qyy(q.y * q.y);
    float qzz(q.z * q.z);
    float qxz(q.x * q.z);
    float qxy(q.x * q.y);
    float qyz(q.y * q.z);
    float qwx(q.w * q.x);
    float qwy(q.w * q.y);
    float qwz(q.w * q.z);

	Result[0][0] = 1 - 2 * (qyy + qzz);
	Result[1][0] = 2 * (qxy - qwz);
	Result[2][0] = 2 * (qxz + qwy);
	Result[0][1] = 2 * (qxy + qwz);
	Result[1][1] = 1 - 2 * (qxx + qzz);
	Result[2][1] = 2 * (qyz - qwx);
	Result[0][2] = 2 * (qxz - qwy);
	Result[1][2] = 2 * (qyz + qwx);
	Result[2][2] = 1 - 2 * (qxx + qyy);
	return Result;  
}

vkm::mat4 vkm::mat4_cast(const quat& q)
{
	return mat4(mat3_cast(q));
}

vkm::mat4 vkm::inverse(const mat4& m)
{
    float Coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
    float Coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
    float Coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

    float Coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
    float Coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
    float Coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

    float Coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
    float Coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
    float Coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

    float Coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
    float Coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
    float Coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

    float Coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
    float Coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
    float Coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

    float Coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
    float Coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
    float Coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

    vec4 Fac0(Coef00, Coef00, Coef02, Coef03);
    vec4 Fac1(Coef04, Coef04, Coef06, Coef07);
    vec4 Fac2(Coef08, Coef08, Coef10, Coef11);
    vec4 Fac3(Coef12, Coef12, Coef14, Coef15);
    vec4 Fac4(Coef16, Coef16, Coef18, Coef19);
    vec4 Fac5(Coef20, Coef20, Coef22, Coef23);
        
    vec4 Vec0(m[1][0], m[0][0], m[0][0], m[0][0]);
    vec4 Vec1(m[1][1], m[0][1], m[0][1], m[0][1]);
    vec4 Vec2(m[1][2], m[0][2], m[0][2], m[0][2]);
    vec4 Vec3(m[1][3], m[0][3], m[0][3], m[0][3]);
        
    vec4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
    vec4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
    vec4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
    vec4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);
        
    vec4 SignA(+1, -1, +1, -1);
    vec4 SignB(-1, +1, -1, +1);
    mat4 Inverse = mat4();

    Inverse[0] = Inv0 * SignA;
    Inverse[1] = Inv1 * SignB;
    Inverse[2] = Inv2 * SignA;
    Inverse[3] = Inv3 * SignB;

    vec4 Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

    vec4 Dot0(m[0] * Row0);
    float Dot1 = (Dot0.x() + Dot0.y()) + (Dot0.z() + Dot0.w());

    float OneOverDeterminant = 1.f / Dot1;

    return Inverse * OneOverDeterminant;
}
