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

    class JsonNull {
    public:
        std::string serialize() const { return "null"; }
    };

    class JsonValue
        : public std::variant<JsonNull, std::string, JsonObject, int, float, JsonArray> {
    public:
        using std::variant<JsonNull, std::string, JsonObject, int, float,
            JsonArray>::variant; // Inherit constructors

        JsonValue(std::initializer_list<std::pair<const std::string, JsonValue>> init)
            : std::variant<JsonNull, std::string, JsonObject, int, float, JsonArray>(
                JsonObject(init))
        {
        }

        std::string serialize(bool beautify = false, int indentLevel = 0) const;

        const JsonValue& operator[](size_t index) const;
        const JsonValue& operator[](const std::string& key) const;
        // JsonValue& operator[](const char * key);

        void printType() const;

    public:
        static JsonValue parseJsonFromString(const std::string& jsonStr);
        static JsonValue parseJsonFromFile(const std::string& path);
        static bool saveJsonToString(const JsonValue& json, std::string& str);
        static bool saveJsonToFile(const JsonValue& json, const std::string& path);

    public:
        bool isNull() const { return std::holds_alternative<JsonNull>(*this); }
        bool isString() const { return std::holds_alternative<std::string>(*this); }
        bool isObject() const { return std::holds_alternative<JsonObject>(*this); }
        bool isInt() const { return std::holds_alternative<int>(*this); }
        bool isFloat() const { return std::holds_alternative<float>(*this); }
        bool isArray() const { return std::holds_alternative<JsonArray>(*this); }

    public:
        const std::string& getString() const
        {
            if (!isString()) {
                throw std::bad_variant_access {};
            }
            return std::get<std::string>(*this);
        }

        const JsonObject& getObject() const
        {
            if (!isObject()) {
                throw std::bad_variant_access {};
            }
            return std::get<JsonObject>(*this);
        }

        int getInt() const
        {
            if (!isInt()) {
                throw std::bad_variant_access {};
            }
            return std::get<int>(*this);
        }

        float getFloat() const
        {
            if (!isFloat() && !isInt()) {
                throw std::bad_variant_access {};
            }
            if (isInt())
                return static_cast<float>(std::get<int>(*this));
            else
                return std::get<float>(*this);
        }

        const JsonArray& getArray() const
        {
            if (!isArray()) {
                throw std::bad_variant_access {};
            }
            return std::get<JsonArray>(*this);
        }

        const std::vector<float> getVecFloat() const
        {
            if (!isArray()) {
                throw std::runtime_error("JsonValue is not an array");
            }
            std::vector<float> vec;
            for (const auto& value : std::get<JsonArray>(*this)) {
                vec.push_back(value.getFloat());
            }
            return vec;
        }

    public:
        // Retrieve optional string
        std::optional<std::string> getOptionalString() const
        {
            if (const auto* value = std::get_if<std::string>(this)) {
                return *value;
            }
            return std::nullopt;
        }

        // Retrieve optional JsonObject
        std::optional<JsonObject> getOptionalObject() const
        {
            if (const auto* value = std::get_if<JsonObject>(this)) {
                return *value;
            }
            return std::nullopt;
        }

        // Retrieve optional int
        std::optional<int> getOptionalInt() const
        {
            if (const auto* value = std::get_if<int>(this)) {
                return *value;
            }
            return std::nullopt;
        }

        // Retrieve optional float
        std::optional<float> getOptionalFloat() const
        {
            if (const auto* value = std::get_if<float>(this)) {
                return *value;
            }
            return std::nullopt;
        }

        // Retrieve optional JsonArray
        std::optional<JsonArray> getOptionalArray() const
        {
            if (const auto* value = std::get_if<JsonArray>(this)) {
                return *value;
            }
            return std::nullopt;
        }

    private:
        static void skipWhitespace(const std::string& json, size_t& pos);
        static std::string extractString(const std::string& json, size_t& pos);
        static JsonObject parseObject(const std::string& json, size_t& pos);
        static JsonArray parseArray(const std::string& json, size_t& pos);
        static JsonValue parseNumber(const std::string& json, size_t& pos);
        static JsonValue parseValue(const std::string& json, size_t& pos);
    };

} // namespace json
} // namespace Utility
