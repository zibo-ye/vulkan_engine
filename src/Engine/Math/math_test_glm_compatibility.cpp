#include "pch.hpp"
#include "math.hpp"

#if USE_GLM

namespace vkm {
template <typename T, std::size_t L>
bool compare_vkm_glm_vec(const vkm::vec_base<L, T>& vkm_vec, const glm::vec<L, T>& glm_vec, float epsilon = std::numeric_limits<T>::epsilon())
{
    for (size_t i = 0; i < L; ++i) {
        if (std::abs(vkm_vec[i] - glm_vec[i]) > epsilon) {
            return false;
        }
    }
    return true;
}

// Function to compare GLM and VKM quaternions
template <typename T>
bool compare_quaternions(const glm::qua<T>& gQuat, const vkm::qua<T>& vQuat, T epsilon = std::numeric_limits<T>::epsilon())
{
    return std::abs(gQuat.x - vQuat.x) < epsilon && std::abs(gQuat.y - vQuat.y) < epsilon && std::abs(gQuat.z - vQuat.z) < epsilon && std::abs(gQuat.w - vQuat.w) < epsilon;
}

template <typename T, std::size_t C, std::size_t R>
bool compare_matrices(const glm::mat<C, R, T>& glmMat, const vkm::mat<C, R, T>& vkmMat, T epsilon = std::numeric_limits<T>::epsilon())
{
    for (std::size_t col = 0; col < C; ++col) {
        for (std::size_t row = 0; row < R; ++row) {
            if (std::fabs(glmMat[col][row] - vkmMat[col][row]) > epsilon) {
                return false;
            }
        }
    }
    return true;
}
    
void test_vkm_glm_quat()
{
    std ::cout << "Running VKM quaternion tests..." << std::endl;

    // Define an axis and an angle for the rotation
    glm::vec3 axis(0.0f, 1.0f, 0.0f);
    float angle = glm::radians(90.0f); // Convert degrees to radians

    // Create a quaternion from axis-angle using both GLM and VKM
    glm::quat glmQuat(glm::angleAxis(angle, axis));
    vkm::quat vkmQuat(angle, vkm::vec<3, float>(axis.x, axis.y, axis.z));

    // Compare the quaternions
    if (!compare_quaternions(glmQuat, vkmQuat)) {
        std::cerr << "Quaternion creation test failed" << std::endl;
        std::cerr << "GLM quat: " << glm::to_string(glmQuat) << std::endl;
        std::cerr << "VKM quat: (" << vkmQuat.x << ", " << vkmQuat.y << ", " << vkmQuat.z << ", " << vkmQuat.w << ")" << std::endl;
    }

    // Normalize the quaternions
    glmQuat = glm::normalize(glmQuat);
    vkmQuat.normalize();

    // Compare the normalized quaternions
    if (!compare_quaternions(glmQuat, vkmQuat)) {
        std::cerr << "Quaternion normalization test failed" << std::endl;
    }

    // Combine rotations by quaternion multiplication
    glm::quat glmQuat2 = glmQuat * glmQuat;
    vkm::quat vkmQuat2 = vkmQuat * vkmQuat;

    // Compare the combined quaternions
    if (!compare_quaternions(glmQuat2, vkmQuat2)) {
        std::cerr << "Quaternion multiplication test failed" << std::endl;
    }

    { // SLERP test
        // First quaternion: Rotation of 45 degrees around the Y axis
        glm::quat glmQ1 = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));
        vkm::qua<float> vkmQ1 = vkm::qua<float>(glm::radians(45.0f), vkm::vec<3, float>(0, 1, 0));

        // Second quaternion: Rotation of 45 degrees around the X axis
        glm::quat glmQ2 = glm::angleAxis(glm::radians(45.0f), glm::vec3(1, 0, 0));
        vkm::qua<float> vkmQ2 = vkm::qua<float>(glm::radians(45.0f), vkm::vec<3, float>(1, 0, 0));

        bool passed = true;

        for (int i = 0; i < 100; i++) {
            // Interpolation factor
            float t = i / 100.0f;

            // Perform slerp interpolation
            glm::quat glmResult = glm::slerp(glmQ1, glmQ2, t);
            vkm::qua<float> vkmResult = vkm::slerp(vkmQ1, vkmQ2, t);

            // Compare the results
            if (!compare_quaternions(glmResult, vkmResult, 1.0e-6f)) {
                passed = false;
                std::cerr << "SLERP test failed on t=" << t << std::endl;
                std::cerr << "GLM quat: " << glm::to_string(glmResult) << std::endl;
                std::cerr << "VKM quat: (" << vkmResult.x << ", " << vkmResult.y << ", " << vkmResult.z << ", " << vkmResult.w << ")" << std::endl;
            }
        }

        if (passed) {
            std::cout << "SLERP test passed" << std::endl;
        } else
            std::cerr << "SLERP test failed" << std::endl;
    }

    std::cout << "All tests completed." << std::endl;
}

void test_vkm_glm_vec()
{
    std::cout << "Running VKM vector tests..." << std::endl;
    // Test vec1
    vkm::vec1 vkm_v1(1.0f);
    glm::vec1 glm_v1(1.0f);
    vkm_v1 += vkm::vec1(1.0f);
    glm_v1 += glm::vec1(1.0f);
    if (!compare_vkm_glm_vec(vkm_v1, glm_v1)) {
        std::cerr << "vec1 addition test failed" << std::endl;
    }

    // Test vec2
    vkm::vec2 vkm_v2(1.0f, 2.0f);
    glm::vec2 glm_v2(1.0f, 2.0f);
    vkm_v2 -= vkm::vec2(0.5f, 0.5f);
    glm_v2 -= glm::vec2(0.5f, 0.5f);
    if (!compare_vkm_glm_vec(vkm_v2, glm_v2)) {
        std::cerr << "vec2 subtraction test failed" << std::endl;
    }

    // Test vec3
    vkm::vec3 vkm_v3(1.0f, 2.0f, 3.0f);
    glm::vec3 glm_v3(1.0f, 2.0f, 3.0f);
    vkm_v3 *= vkm::vec3(2.0f, 2.0f, 2.0f);
    glm_v3 *= glm::vec3(2.0f, 2.0f, 2.0f);
    if (!compare_vkm_glm_vec(vkm_v3, glm_v3)) {
        std::cerr << "vec3 multiplication test failed" << std::endl;
    }

    // Test vec4
    vkm::vec4 vkm_v4(1.0f, 2.0f, 3.0f, 4.0f);
    glm::vec4 glm_v4(1.0f, 2.0f, 3.0f, 4.0f);
    vkm_v4 /= vkm::vec4(1.0f, 2.0f, 3.0f, 4.0f);
    glm_v4 /= glm::vec4(1.0f, 2.0f, 3.0f, 4.0f);
    if (!compare_vkm_glm_vec(vkm_v4, glm_v4)) {
        std::cerr << "vec4 division test failed" << std::endl;
    }

    std::cout << "All tests completed." << std::endl;
}

void testMatrixConstruction()
{
    glm::mat4 glmMat(1.0); // GLM identity matrix
    vkm::mat4 vkmMat; // Your VKM matrix, assuming default constructor creates an identity matrix

    if (!compare_matrices(glmMat, vkmMat)) {
        std::cerr << "Matrix construction test failed." << std::endl;
    } else {
        std::cout << "Matrix construction test passed." << std::endl;
    }
}
void testMatrixMultiplication()
{
    glm::mat4 glmMat1(1.0);
    glm::mat4 glmMat2 = glm::translate(glmMat1, glm::vec3(1.0, 2.0, 3.0));

    vkm::mat4 vkmMat1; // Assuming default constructor creates an identity matrix
    vkm::mat4 vkmMat2 = vkmMat1 * vkm::translate(vkm::vec3(1.0, 2.0, 3.0)); // Implement your translate function

    if (!compare_matrices(glmMat2, vkmMat2)) {
        std::cerr << "Matrix multiplication test failed." << std::endl;
    } else {
        std::cout << "Matrix multiplication test passed." << std::endl;
    }
}
void testMatrixVectorMultiplication()
{
    glm::mat4 glmMat(1.0);
    glm::vec4 glmVec(1.0, 2.0, 3.0, 1.0);
    glm::vec4 glmResult = glmMat * glmVec;

    vkm::mat4 vkmMat; // Assuming default constructor creates an identity matrix
    vkm::vec4 vkmVec(1.0, 2.0, 3.0, 1.0);
    vkm::vec4 vkmResult = vkmMat * vkmVec;

    if (!compare_vkm_glm_vec(vkmResult, glmResult)) { // Implement compare_vectors similar to compare_matrices
        std::cerr << "Matrix-vector multiplication test failed." << std::endl;
    } else {
        std::cout << "Matrix-vector multiplication test passed." << std::endl;
    }
}

void test_vkm_glm_mat()
{
    testMatrixConstruction();
    testMatrixMultiplication();
    testMatrixVectorMultiplication();
}

void testPerspective()
{
    float fovY = glm::radians(45.0f);
    float aspect = 4.0f / 3.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;

    auto glmMat = glm::perspective(fovY, aspect, zNear, zFar);
    glmMat[1][1] *= -1; // Flip the Y axis
    auto vkmMat = vkm::perspective(fovY, aspect, zNear, zFar);

    assert(compare_matrices(glmMat, vkmMat));
}

void testLookAt()
{
    glm::vec3 glm_eye(1.0f, 2.0f, 3.0f);
    glm::vec3 glm_center(0.0f, 0.0f, 0.0f);
    glm::vec3 glm_up(0.0f, 1.0f, 0.0f);
    vkm::vec3 vkm_eye(1.0f, 2.0f, 3.0f);
    vkm::vec3 vkm_center(0.0f, 0.0f, 0.0f);
    vkm::vec3 vkm_up(0.0f, 1.0f, 0.0f);

    auto glmMat = glm::lookAt(glm_eye, glm_center, glm_up);
    auto vkmMat = vkm::lookAt(vkm_eye, vkm_center, vkm_up);

    assert(compare_matrices(glmMat, vkmMat));
}

void testScale()
{
    glm::mat4 m = glm::mat4(1.0f);
    glm::vec3 v(2.0f, 3.0f, 4.0f);
    vkm::vec3 vkm_v(2.0f, 3.0f, 4.0f);
    vkm::mat4 vkmMat;

    auto glmMat = glm::scale(m, v);
    vkmMat = vkm::scale(vkmMat, vkm_v);

    assert(compare_matrices(glmMat, vkmMat));
}

void testTranslate()
{
    glm::vec3 v(1.0f, 2.0f, 3.0f);
    vkm::vec3 vkm_v(1.0f, 2.0f, 3.0f);

    auto glmMat = glm::translate(glm::mat4(1.0f), v);
    auto vkmMat = vkm::translate(vkm_v);

    assert(compare_matrices(glmMat, vkmMat));
}

void test_vkm_glm_ops()
{
    std::cout << "Running VKM matrix tests..." << std::endl;
    testPerspective();
    testLookAt();
    testScale();
    testTranslate();
    std::cout << "All tests completed." << std::endl;
}


void test_vkm_glm_compatibility()
{
    test_vkm_glm_vec();
    test_vkm_glm_quat();
    test_vkm_glm_mat();
    test_vkm_glm_ops();
}

}

#endif