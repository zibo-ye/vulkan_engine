#pragma once

#include "pch.hpp"

#include <cctype> // for std::isspace
#include <iostream>
#include <sstream>
#include <stdexcept> // for std::runtime_error
#include <string>
#include <unordered_map>
#include <variant>

namespace Utility {
namespace json {

    class JsonValue;

    using JsonArray = std::vector<JsonValue>;
    using JsonObject = std::unordered_map<std::string, JsonValue>;

    class JsonValue
        : public std::variant<std::string, JsonObject, int, float, JsonArray> {
    public:
        using std::variant<std::string, JsonObject, int, float,
            JsonArray>::variant; // Inherit constructors

        JsonValue(std::initializer_list<std::pair<const std::string, JsonValue>> init)
            : std::variant<std::string, JsonObject, int, float, JsonArray>(
                JsonObject(init))
        {
        }

        std::string serialize(bool beautify = false, int indentLevel = 0) const;

        JsonValue& operator[](size_t index);

        JsonValue& operator[](const std::string& key);

        void printType() const;

        static JsonValue parseJson(const std::string& json);
    };

    void skipWhitespace(const std::string& json, size_t& pos);

    std::string extractString(const std::string& json, size_t& pos);

    JsonObject parseObject(const std::string& json, size_t& pos);

    JsonArray parseArray(const std::string& json, size_t& pos);

    JsonValue parseNumber(const std::string& json, size_t& pos);

    JsonValue parseValue(const std::string& json, size_t& pos);

} // namespace json
} // namespace Utility
