#pragma once
#include <array>
#include <cstdlib>

namespace vkm {
using std::size_t;
// template <std::size_t C, std::size_t R, typename T> struct mat;
// template <typename T> struct qua;

template <std::size_t L, typename T>
struct vec_base {
    static_assert(L > 0, "Size of vec must be greater than 0");

    typedef T value_type;
    std::array<T, L> data; // memory layout same as T[L]

    T& operator[](std::size_t index)
    {
        return data[index];
    }

    const T& operator[](std::size_t index) const
    {
        return data[index];
    }

    // Element-wise addition using a for loop
    vec_base<L, T>& operator+=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] += other[i];
        }
        return *this;
    }

    // Element-wise subtraction using a for loop
    vec_base<L, T>& operator-=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] -= other[i];
        }
        return *this;
    }

    // Element-wise multiplication using a for loop
    vec_base<L, T>& operator*=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] *= other[i];
        }
        return *this;
    }

    // Element-wise division using a for loop
    vec_base<L, T>& operator/=(const vec_base<L, T>& other)
    {
        for (std::size_t i = 0; i < L; ++i) {
            data[i] /= other[i];
        }
        return *this;
    }
};

// General vec template
template <std::size_t L, typename T>
struct vec : public vec_base<L, T> {
};

// Specialization for L=1
template <typename T>
struct vec<1, T> : public vec_base<1, T> {
    T& x() { return this->data[0]; }
    T& r() { return this->data[0]; }
    const T& x() const { return this->data[0]; }
    const T& r() const { return this->data[0]; }
};

// Specialization for L=2
template <typename T>
struct vec<2, T> : public vec_base<2, T> {
    T& x() { return this->data[0]; }
    T& r() { return this->data[0]; }
    T& y() { return this->data[1]; }
    T& g() { return this->data[1]; }
};

// Specialization for L=3
template <typename T>
struct vec<3, T> : public vec_base<3, T> {
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

// Trying to use enable_if, still buggy
/*
template <std::size_t L, typename T>
struct vec : public vec_base<L, T> {
    template <std::size_t L, std::size_t N>
    using enable_if_ge = std::enable_if_t<(L >= N), T&>;

    // Use enable_if_ge to conditionally enable these member functions
    enable_if_ge<L, 1> x() { return this->data[0]; }
    enable_if_ge<L, 1> r() { return this->data[0]; }
    enable_if_ge<L, 2> y() { return this->data[1]; }
    enable_if_ge<L, 2> g() { return this->data[1]; }
    enable_if_ge<L, 3> z() { return this->data[2]; }
    enable_if_ge<L, 3> b() { return this->data[2]; }
    enable_if_ge<L, 4> w() { return this->data[3]; }
    enable_if_ge<L, 4> a() { return this->data[3]; }

    // Const versions
    enable_if_ge<L, 1> x() const { return this->data[0]; }
    enable_if_ge<L, 1> r() const { return this->data[0]; }
    enable_if_ge<L, 2> y() const { return this->data[1]; }
    enable_if_ge<L, 2> g() const { return this->data[1]; }
    enable_if_ge<L, 3> z() const { return this->data[2]; }
    enable_if_ge<L, 3> b() const { return this->data[2]; }
    enable_if_ge<L, 4> w() const { return this->data[3]; }
    enable_if_ge<L, 4> a() const { return this->data[3]; }
};
*/

typedef vec<1, float> vec1;
typedef vec<2, float> vec2;
typedef vec<3, float> vec3;
typedef vec<4, float> vec4;
}
