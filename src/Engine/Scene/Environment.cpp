#include "Environment.hpp"
#include "Utilities/lambertian/blur_cube.h"
#include <random>

Environment::Environment(std::weak_ptr<Scene> pScene, size_t index, const Utility::json::JsonValue& jsonObj)
    : SceneObj(pScene, index, ESceneObjType::ENVIRONMENT)
{
    name = jsonObj["name"].getString();

    radiance = Texture(jsonObj["radiance"], pScene.lock()->src);

    lambertian = GenerateLambertian2(radiance);
    // lambertian = GenerateLambertian(radiance); // currently not producing the same result as GenerateLambertian2
}

#define M_PI 3.14159265358979323846f

enum class CubeMapFace {
    PositiveX = 0,
    NegativeX = 1,
    PositiveY = 2,
    NegativeY = 3,
    PositiveZ = 4,
    NegativeZ = 5
};

inline vkm::vec3 rgbe_to_float(vkm::u8vec4 col)
{
    // avoid decoding zero to a denormalized value:
    if (col == vkm::u8vec4(0, 0, 0, 0))
        return vkm::vec3(0.0f);

    int exp = int(col.a()) - 128;
    return vkm::vec3(
        std::ldexp((col.r() + 0.5f) / 256.0f, exp),
        std::ldexp((col.g() + 0.5f) / 256.0f, exp),
        std::ldexp((col.b() + 0.5f) / 256.0f, exp));
}

inline vkm::u8vec4 float_to_rgbe(vkm::vec3 col)
{

    float d = std::max(col.r(), std::max(col.g(), col.b()));

    // 1e-32 is from the radiance code, and is probably larger than strictly necessary:
    if (d <= 1e-32f) {
        return vkm::u8vec4(0, 0, 0, 0);
    }

    int e;
    float fac = 255.999f * (std::frexp(d, &e) / d);

    // value is too large to represent, clamp to bright white:
    if (e > 127) {
        return vkm::u8vec4(0xff, 0xff, 0xff, 0xff);
    }

    // scale and store:
    return vkm::u8vec4(
        std::max(0, int32_t(col.r() * fac)),
        std::max(0, int32_t(col.g() * fac)),
        std::max(0, int32_t(col.b() * fac)),
        e + 128);
}

// Adopted from blur_cube.cpp: https://github.com/ixchow/15-466-ibl/blob/master/cubes/blur_cube.cpp
Texture GenerateLambertian(Texture radiance, int lambertian_texWidth /*= 16*/)
{
    Texture lambertian = Texture();
    lambertian.type = radiance.type;
    lambertian.format = radiance.format;

    lambertian.texWidth = lambertian_texWidth;
    lambertian.texHeight = lambertian_texWidth;
    lambertian.texChannels = radiance.texChannels;
    lambertian.mipLevels = 1;

    stbi_uc* rawData = new stbi_uc[4 * lambertian_texWidth * lambertian_texWidth * 6];
    lambertian.textureData = std::shared_ptr<stbi_uc>(rawData, [](stbi_uc* p) { delete[] p; });

    // Define sampling and bright direction handling as in the provided snippet
    std::function<vkm::vec3()> make_sample;

    // Initialize sampling based on diffuse mode (as an example)
    make_sample = []() -> vkm::vec3 {
        static std::mt19937 mt(0x12341234);
        vkm::vec2 rv(mt() / float(mt.max()), mt() / float(mt.max()));
        float phi = rv.x() * 2.0f * M_PI;
        float r = std::sqrt(rv.y());
        return vkm::vec3(
            std::cos(phi) * r,
            std::sin(phi) * r,
            std::sqrt(1.0f - rv.y()));
    };

    // Convert radiance to linear data
    std::vector<vkm::vec3> linearData(radiance.texWidth * radiance.texHeight * 6);

    for (int i = 0; i < radiance.texWidth * radiance.texHeight * 6; ++i) {
        vkm::u8vec4 rgbe = vkm::u8vec4(
            radiance.textureData.get()[4 * i + 0],
            radiance.textureData.get()[4 * i + 1],
            radiance.textureData.get()[4 * i + 2],
            radiance.textureData.get()[4 * i + 3]);
        linearData[i] = rgbe_to_float(rgbe);
    }

    int in_size = radiance.texWidth;

    // function for sampling a given direction from cubemap:
    auto lookup = [&linearData, &in_size](vkm::vec3 const& dir) -> vkm::vec3 {
        float sc, tc, ma;
        uint32_t f;
        if (std::abs(dir.x()) >= std::abs(dir.y()) && std::abs(dir.x()) >= std::abs(dir.z())) {
            if (dir.x() >= 0) {
                sc = -dir.z();
                tc = -dir.y();
                ma = dir.x();
                f = static_cast<uint32_t>(CubeMapFace::PositiveX);
            } else {
                sc = dir.z();
                tc = -dir.y();
                ma = -dir.x();
                f = static_cast<uint32_t>(CubeMapFace::NegativeX);
            }
        } else if (std::abs(dir.y()) >= std::abs(dir.z())) {
            if (dir.y() >= 0) {
                sc = dir.x();
                tc = dir.z();
                ma = dir.y();
                f = static_cast<uint32_t>(CubeMapFace::PositiveY);
            } else {
                sc = dir.x();
                tc = -dir.z();
                ma = -dir.y();
                f = static_cast<uint32_t>(CubeMapFace::NegativeY);
            }
        } else {
            if (dir.z() >= 0) {
                sc = dir.x();
                tc = -dir.y();
                ma = dir.z();
                f = static_cast<uint32_t>(CubeMapFace::PositiveZ);
            } else {
                sc = -dir.x();
                tc = -dir.y();
                ma = -dir.z();
                f = static_cast<uint32_t>(CubeMapFace::NegativeZ);
            }
        }

        int32_t s = static_cast<int32_t>(std::floor(0.5f * (sc / ma + 1.0f) * in_size));
        s = std::max(0, std::min(int32_t(in_size) - 1, s));
        int32_t t = static_cast<int32_t>(std::floor(0.5f * (tc / ma + 1.0f) * in_size));
        t = std::max(0, std::min(int32_t(in_size) - 1, t));

        return linearData[(f * in_size + t) * in_size + s];
    };

    for (int i = 0; i < 6; i++) {
        // std::cout << "Sampling face " << i << "/6 ..." << std::endl;
        vkm::vec3 sc; // maps to rightward axis on face
        vkm::vec3 tc; // maps to upward axis on face
        vkm::vec3 ma; // direction to face
        CubeMapFace face = static_cast<CubeMapFace>(i);
        if (face == CubeMapFace::PositiveX) {
            sc = vkm::vec3(0.0f, 0.0f, -1.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(1.0f, 0.0f, 0.0f);
        } else if (face == CubeMapFace::NegativeX) {
            sc = vkm::vec3(0.0f, 0.0f, 1.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(-1.0f, 0.0f, 0.0f);
        } else if (face == CubeMapFace::PositiveY) {
            sc = vkm::vec3(1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, 0.0f, 1.0f);
            ma = vkm::vec3(0.0f, 1.0f, 0.0f);
        } else if (face == CubeMapFace::NegativeY) {
            sc = vkm::vec3(1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, 0.0f, -1.0f);
            ma = vkm::vec3(0.0f, -1.0f, 0.0f);
        } else if (face == CubeMapFace::PositiveZ) {
            sc = vkm::vec3(1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(0.0f, 0.0f, 1.0f);
        } else if (face == CubeMapFace::NegativeZ) {
            sc = vkm::vec3(-1.0f, 0.0f, 0.0f);
            tc = vkm::vec3(0.0f, -1.0f, 0.0f);
            ma = vkm::vec3(0.0f, 0.0f, -1.0f);
        } else
            assert(0 && "Invalid face.");

        for (int y = 0; y < lambertian_texWidth; y++) {
            for (int x = 0; x < lambertian_texWidth; x++) {

                vkm::vec3 N = (ma
                    + (2.0f * (x + 0.5f) / lambertian_texWidth - 1.0f) * sc
                    + (2.0f * (y + 0.5f) / lambertian_texWidth - 1.0f) * tc);
                N = vkm::normalize(N);
                vkm::vec3 temp = (abs(N.z()) < 0.99f ? vkm::vec3(0.0f, 0.0f, 1.0f) : vkm::vec3(1.0f, 0.0f, 0.0f));
                vkm::vec3 TX = vkm::normalize(vkm::cross(N, temp));
                vkm::vec3 TY = vkm::cross(N, TX);

                vkm::vec3 acc = vkm::vec3(0.0f);
                int samples = 1000;
                for (uint32_t i = 0; i < uint32_t(samples); ++i) {
                    // very inspired by the SampleGGX code in "Real Shading in Unreal" (https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf):

                    vkm::vec3 dir = make_sample();

                    acc += lookup(dir.x() * TX + dir.y() * TY + dir.z() * N);
                    // acc += (dir.x * TX + dir.y * TY + dir.z * N) * 0.5f + 0.5f; //DEBUG
                }

                acc *= 1.0f / float(samples);

                // convert to RGBE:
                vkm::u8vec4 rgbe = float_to_rgbe(acc);

                // store in the texture:
                int index = 4 * (x + y * lambertian_texWidth + i * lambertian_texWidth * lambertian_texWidth);
                lambertian.textureData.get()[index + 0] = rgbe.r();
                lambertian.textureData.get()[index + 1] = rgbe.g();
                lambertian.textureData.get()[index + 2] = rgbe.b();
                lambertian.textureData.get()[index + 3] = rgbe.a();
            }
        }
    }
    return lambertian;
}

Texture GenerateLambertian2(Texture radiance)
{
    glm::ivec2 out_size = glm::ivec2(16, 16 * 6);
    int32_t samples = 1024;
    int32_t brightest = 10000;
    std::string in_file = radiance.src;

    // outfilename = in_file's filename + "_lambertian" + in_file's extension
    std::string out_file = in_file.substr(0, in_file.find_last_of('.')) + "_lambertian" + in_file.substr(in_file.find_last_of('.'));

    blur_cube("diffuse", out_size, samples, in_file, brightest, out_file);

    Texture lambertian = Texture();
    lambertian.src = out_file;
    lambertian.type = radiance.type;
    lambertian.format = radiance.format;
    lambertian.LoadTextureData();

    return std::move(lambertian);
}
