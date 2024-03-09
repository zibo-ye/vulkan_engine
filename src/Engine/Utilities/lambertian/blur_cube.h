#pragma once

#include <string>
#include <glm/glm.hpp>

void blur_cube(std::string mode, glm::ivec2& out_size, int32_t& samples, std::string& in_file, int32_t brightest, std::string out_file);