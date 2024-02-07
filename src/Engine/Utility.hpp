#pragma once

#include "pch.hpp"

namespace Utility {
namespace json {
    class json_parser {
    public:
        
        void read_from(std::string& str);

    };

    class json_value {
        std::unordered_map<std::string, std::string> data;

    };
}
} // namespace Utility
