#pragma once
#include "pch.hpp"

class Texture {
public:
    std::string src;
    std::string type;
    std::string format;
    Texture() = default;
    Texture(const Utility::json::JsonValue& jsonObj)
    {
        src = jsonObj["src"].getString();
        type = jsonObj.hasKey("type") ? jsonObj["type"].getString() : "2D"; // Default to "2D" 
        format = jsonObj.hasKey("format") ? jsonObj["format"].getString() : "linear"; // Default to "linear"
    }
};
