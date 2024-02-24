#pragma once
#include "pch.hpp"
#include <array>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace vkm {
using std::size_t;
template <std::size_t L, typename T>
struct vec;

template <std::size_t L, typename T>
struct vec_base {
    static_assert(L > 0, "Size of vec must be greater than 0");

    typedef T value_type;
    std::array<T, L> data; // memory layout same as T[L]

    vec_base()
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] = T(0);
        }
    }

    vec_base(std::initializer_list<T> init)
    {
        std::copy(init.begin(), init.end(), data.begin());
    }

    vec_base(T value)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] = value;
        }
    }

    vec_base(const vec_base<L, T>& other)
        : data(other.data)
    {
    }

    T& operator[](std::size_t index)
    {
        return data[index];
    }

    const T& operator[](std::size_t index) const
    {
        return data[index];
    }

    operator vec<L, T>() const
    {
        return vec<L, T>(*this);
    }

    vec_base operator-() const
    {
        vec_base result;
        for (std::size_t i = 0; i < L; ++i) {
            result[i] = -data[i];
        }
        return result;
    }

    vec_base<L, T>& operator+=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] += other[i];
        }
        return *this;
    }

    vec_base<L, T>& operator-=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] -= other[i];
        }
        return *this;
    }

    vec_base<L, T>& operator*=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] *= other[i];
        }
        return *this;
    }

    vec_base<L, T>& operator*=(const T& scalar)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] *= scalar;
        }
        return *this;
    }

    vec_base<L, T>& operator/=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] /= other[i];
        }
        return *this;
    }

    vec_base<L, T>& operator/=(const T& scalar)
    {
        assert(scalar != T(0) && "Division by zero scalar in vector division");
        for (std::size_t i = 0; i < L; ++i) {
            data[i] /= scalar;
        }
        return *this;
    }

    bool operator==(const vec_base<L, T>& other) const
    {
        for (std::size_t i = 0; i < L; ++i) {
            if (data[i] != other.data[i]) {
                return false;
            }
        }
        return true;
    }
};

template <std::size_t L, typename T>
vec_base<L, T> operator+(const vec_base<L, T>& lhs, const vec_base<L, T>& rhs)
{
    vec_base<L, T> result;
    for (std::size_t i = 0; i < L; ++i) {
        result[i] = lhs[i] + rhs[i];
    }
    return result;
}

template <std::size_t L, typename T>
vec_base<L, T> operator-(const vec_base<L, T>& lhs, const vec_base<L, T>& rhs)
{
    vec_base<L, T> result;
    for (std::size_t i = 0; i < L; ++i) {
        result[i] = lhs[i] - rhs[i];
    }
    return result;
}

template <std::size_t L, typename T>
vec_base<L, T> operator*(const vec_base<L, T>& lhs, const vec_base<L, T>& rhs)
{
    vec_base<L, T> result;
    for (std::size_t i = 0; i < L; ++i) {
        result[i] = lhs[i] * rhs[i];
    }
    return result;
}

template <std::size_t L, typename T>
vec_base<L, T> operator/(const vec_base<L, T>& lhs, const vec_base<L, T>& rhs)
{
    vec_base<L, T> result;
    for (std::size_t i = 0; i < L; ++i) {
        // Ensure there's no division by zero
        assert(rhs[i] != T(0) && "Division by zero!");
        result[i] = lhs[i] / rhs[i];
    }
    return result;
}

template <std::size_t L, typename T>
vec_base<L, T> operator*(const vec_base<L, T>& vec, const T& scalar)
{
    vec_base<L, T> result;
    for (std::size_t i = 0; i < L; ++i) {
        result[i] = vec[i] * scalar;
    }
    return result;
}

template <std::size_t L, typename T>
vec_base<L, T> operator*(const T& scalar, const vec_base<L, T>& vec)
{
    return vec * scalar;
}

template <std::size_t L, typename T>
vec_base<L, T> operator/(const vec_base<L, T>& vec, const T& scalar)
{
    assert(scalar != T(0) && "Division by zero scalar in vector division");

    vec_base<L, T> result;
    for (std::size_t i = 0; i < L; ++i) {
        result[i] = vec[i] / scalar;
    }
    return result;
}

// General vec template
template <std::size_t L, typename T>
struct vec : public vec_base<L, T> {
    vec()
        : vec_base<L, T>()
    {
    }
};

// Specialization for L=1
template <typename T>
struct vec<1, T> : public vec_base<1, T> {
    vec()
        : vec_base<1, T>()
    {
    }
    vec(T x)
        : vec_base<1, T>({ x })
    {
    }
    explicit vec(const vec_base<1, T>& base)
        : vec_base<1, T>(base)
    {
    }
    T& x() { return this->data[0]; }
    T& r() { return this->data[0]; }
    const T& x() const { return this->data[0]; }
    const T& r() const { return this->data[0]; }
};

// Specialization for L=2
template <typename T>
struct vec<2, T> : public vec_base<2, T> {
    vec()
        : vec_base<2, T>()
    {
    }
    vec(T x, T y)
        : vec_base<2, T>({ x, y })
    {
    }
    explicit vec(const vec_base<2, T>& base)
        : vec_base<2, T>(base)
    {
    }
    T& x() { return this->data[0]; }
    T& r() { return this->data[0]; }
    T& y() { return this->data[1]; }
    T& g() { return this->data[1]; }
};

// Specialization for L=3
template <typename T>
struct vec<3, T> : public vec_base<3, T> {
    vec()
        : vec_base<3, T>()
    {
    }
    vec(T x, T y, T z)
        : vec_base<3, T>({ x, y, z })
    {
    }
    explicit vec(const vec_base<3, T>& base)
        : vec_base<3, T>(base)
    {
    }
    T& x() { return this->data[0]; }
    T& r() { return this->data[0]; }
    T& y() { return this->data[1]; }
    T& g() { return this->data[1]; }
    T& z() { return this->data[2]; }
    T& b() { return this->data[2]; }

    const T& x() const { return this->data[0]; }
    const T& r() const { return this->data[0]; }
    const T& y() const { return this->data[1]; }
    const T& g() const { return this->data[1]; }
    const T& z() const { return this->data[2]; }
    const T& b() const { return this->data[2]; }
};

// Specialization for L=4
template <typename T>
struct vec<4, T> : public vec_base<4, T> {
    vec()
        : vec_base<4, T>()
    {
    }
    vec(T x, T y, T z, T w)
        : vec_base<4, T>({ x, y, z, w })
    {
    }
     explicit vec(const vec_base<4, T>& base)
         : vec_base<4, T>(base)
     {
     }
    T& x() { return this->data[0]; }
    T& r() { return this->data[0]; }
    T& y() { return this->data[1]; }
    T& g() { return this->data[1]; }
    T& z() { return this->data[2]; }
    T& b() { return this->data[2]; }
    T& w() { return this->data[3]; }
    T& a() { return this->data[3]; }

    const T& x() const { return this->data[0]; }
    const T& r() const { return this->data[0]; }
    const T& y() const { return this->data[1]; }
    const T& g() const { return this->data[1]; }
    const T& z() const { return this->data[2]; }
    const T& b() const { return this->data[2]; }
    const T& w() const { return this->data[3]; }
    const T& a() const { return this->data[3]; }
};

typedef vec<1, float> vec1;
typedef vec<2, float> vec2;
typedef vec<3, float> vec3;
typedef vec<4, float> vec4;
typedef vec<4, uint8_t> u8vec4;

template <std::size_t N, typename T>
vec<N, T> cross(const vec<N, T>& v1, const vec<N, T>& v2)
{
    static_assert(N == 3, "Cross product is only defined for 3D vectors");
    return vec<3, T> {
        v1[1] * v2[2] - v1[2] * v2[1],
        v1[2] * v2[0] - v1[0] * v2[2],
        v1[0] * v2[1] - v1[1] * v2[0]
    };
}

template <std::size_t N, typename T>
T distance(const vec<N, T>& p1, const vec<N, T>& p2)
{
    T sum = T(0);
    for (std::size_t i = 0; i < N; ++i) {
        T diff = p2[i] - p1[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

template <std::size_t N, typename T>
T length(const vec<N, T>& v)
{
    T sum = T(0);
    for (std::size_t i = 0; i < N; ++i) {
        sum += v[i] * v[i];
    }
    return sqrt(sum);
}

template <std::size_t N, typename T>
T dot(const vec<N, T>& v1, const vec<N, T>& v2)
{
    T sum = T(0);
    for (std::size_t i = 0; i < N; ++i) {
        sum += v1[i] * v2[i];
    }
    return sum;
}

template <std::size_t N, typename T>
vec<N, T> normalize(const vec<N, T>& v)
{
    T len = length(v);
    if (len == 0)
        return v; // Prevent division by zero

    vec<N, T> result;
    for (std::size_t i = 0; i < N; ++i) {
        result[i] = v[i] / len;
    }
    return result;
}

template <typename T>
struct qua {
    T x, y, z, w;

    qua()
        : x(0)
        , y(0)
        , z(0)
        , w(1)
    {
    }

    qua(T x, T y, T z, T w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {
    }

    qua(T angleRadians, const vec<3, T>& axis)
    {
        T halfAngle = angleRadians * T(0.5);
        T s = std::sin(halfAngle);
        x = axis[0] * s;
        y = axis[1] * s;
        z = axis[2] * s;
        w = std::cos(halfAngle);
    }

    T norm() const
    {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    qua& normalize()
    {
        T n = norm();
        if (n > T(0)) {
            T invNorm = T(1) / n;
            x *= invNorm;
            y *= invNorm;
            z *= invNorm;
            w *= invNorm;
        }
        return *this;
    }

    qua conjugate() const
    {
        return qua(-x, -y, -z, w);
    }

    qua inverse() const
    {
        return conjugate().normalize();
    }

    qua operator*(const qua& rhs) const
    {
        return qua(
            w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
            w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
            w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
            w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
    }
};

typedef qua<float> quat;

template <typename T>
qua<T> slerp(const qua<T>& q1, const qua<T>& q2, T t)
{
    // Compute the cosine of the angle between the two vectors.
    T cosTheta = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

    // If q1 is on the opposite hemisphere from q2, invert one of the quaternions.
    if (cosTheta < T(0)) {
        cosTheta = -cosTheta;
    }

    // Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
    if (cosTheta > T(1) - std::numeric_limits<T>::epsilon()) {
        return qua<T>(
            q1.x + t * (q2.x - q1.x),
            q1.y + t * (q2.y - q1.y),
            q1.z + t * (q2.z - q1.z),
            q1.w + t * (q2.w - q1.w))
            .normalize();
    }

    // Compute the sinTheta using the trigonometric identity sin^2(theta) + cos^2(theta) = 1
    T sinTheta = std::sqrt(T(1) - cosTheta * cosTheta);

    // Compute the angle from its cosine and sine.
    T angle = std::atan2(sinTheta, cosTheta);

    T factor1 = std::sin((T(1) - t) * angle) / sinTheta;
    T factor2 = std::sin(t * angle) / sinTheta;

    // Compute the interpolated quaternion.
    return qua<T>(
        factor1 * q1.x + factor2 * q2.x,
        factor1 * q1.y + factor2 * q2.y,
        factor1 * q1.z + factor2 * q2.z,
        factor1 * q1.w + factor2 * q2.w);
}

template <std::size_t C, std::size_t R, typename T>
struct mat {
    static_assert(C > 0 && R > 0, "Matrix dimensions must be greater than 0");

    // Using column-major order: Each column is a vector of length R
    std::array<vec<R, T>, C> data;

    // Default constructor initializes to identity matrix
    mat()
    {
        for (std::size_t col = 0; col < C; ++col) {
            for (std::size_t row = 0; row < R; ++row) {
                data[col][row] = (row == col) ? T(1) : T(0);
            }
        }
    }

    //// Initialize to a scalar value
    // mat(T value)
    //{
    //     for (std::size_t col = 0; col < C; ++col) {
    //         for (std::size_t row = 0; row < R; ++row) {
    //             data[col][row] = value;
    //         }
    //     }
    // }

    // Copy constructor
    mat(const mat& other)
        : data(other.data)
    {
    }

    // Copy from matrix of different size
    template <std::size_t C2, std::size_t R2>
    mat(const mat<C2, R2, T>& other)
    {
        *this = mat();
        for (std::size_t col = 0; col < C && col < C2; ++col) {
            for (std::size_t row = 0; row < R && row < R2; ++row) {
                data[col][row] = other[col][row];
            }
        }
    }

    // Move constructor
    mat(mat&& other) noexcept
        : data(std::move(other.data))
    {
    }

    // = operator
    mat& operator=(const mat& other)
    {
        data = other.data;
        return *this;
    }

    // Access elements using (row, col) notation
    T& operator()(std::size_t row, std::size_t col)
    {
        return data[col][row];
    }

    const T& operator()(std::size_t row, std::size_t col) const
    {
        return data[col][row];
    }

    // Access a column
    vec<R, T>& operator[](std::size_t col)
    {
        return data[col];
    }

    // Access a column (const version)
    const vec<R, T>& operator[](std::size_t col) const
    {
        return data[col];
    }

    // Matrix-Matrix Multiplication
    template <std::size_t C2>
    mat<C2, R, T> operator*(const mat<C2, C, T>& other) const
    {
        mat<C2, R, T> result;
        for (std::size_t i = 0; i < R; ++i) {
            for (std::size_t j = 0; j < C2; ++j) {
                T sum = T(0);
                for (std::size_t k = 0; k < C; ++k) {
                    sum += this->data[k][i] * other.data[j][k];
                }
                result.data[j][i] = sum;
            }
        }
        return result;
    }

    // Matrix-Vector Multiplication
    vec<R, T> operator*(const vec<C, T>& v) const
    {
        vec<R, T> result;
        for (std::size_t i = 0; i < R; ++i) {
            T sum = T(0);
            for (std::size_t j = 0; j < C; ++j) {
                sum += this->data[j][i] * v[j];
            }
            result[i] = sum;
        }
        return result;
    }
};

template <std::size_t C, std::size_t R, typename T>
mat<C, R, T> operator*(const mat<C, R, T>& matrix, const T& scalar)
{
    mat<C, R, T> result;
    for (std::size_t col = 0; col < C; ++col) {
        for (std::size_t row = 0; row < R; ++row) {
            result[col][row] = matrix[col][row] * scalar;
        }
    }
    return result;
}

template <std::size_t C, std::size_t R, typename T>
mat<C, R, T> operator*(const T& scalar, const mat<C, R, T>& matrix)
{
    return matrix * scalar;
}

template <std::size_t C, std::size_t R, typename T>
mat<C, R, T> operator/(const mat<C, R, T>& matrix, const T& scalar)
{
    assert(scalar != T(0) && "Division by zero scalar in matrix division");

    mat<C, R, T> result;
    for (std::size_t col = 0; col < C; ++col) {
        for (std::size_t row = 0; row < R; ++row) {
            result[col][row] = matrix[col][row] / scalar;
        }
    }
    return result;
}

typedef mat<4, 4, float> mat4;
typedef mat<3, 3, float> mat3;

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG2RAD = PI / 180.0f;

inline constexpr float radians(float degrees)
{
    return degrees * DEG2RAD;
}

mat4 perspective(float fovY, float aspect, float zNear, float zFar);
mat4 lookAt(const vec3& eye, const vec3& center, const vec3& up);
mat4 rotate(const mat4& m, float angle, const vec3& axis);
mat4 scale(const mat4& m, const vec3& v);
mat4 translate(const vec3& v);
mat4 translate(const mat4& m, const vec3& v);
vkm::mat3 mat3_cast(const quat& q);
mat4 mat4_cast(const quat& q);

mat4 inverse(const mat4& m);

} // namespace vkm

namespace std {
template <std::size_t L, typename T>
struct hash<vkm::vec<L, T>> {
    size_t operator()(const vkm::vec<L, T>& v) const
    {
        size_t hash = 0;
        for (size_t i = 0; i < L; ++i) {
            hash ^= std::hash<T>()(v.data[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        }
        return hash;
    }
};
} // namespace std
