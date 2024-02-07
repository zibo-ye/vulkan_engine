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

    JsonValue parseValue(const std::string& json, size_t& pos);

    class JsonValue
        : public std::variant<std::string, JsonObject, int, float, JsonArray> {
    public:
        using std::variant<std::string, JsonObject, int, float,
            JsonArray>::variant; // Inherit constructors

        // Constructor for JsonObject initialization
        JsonValue(std::initializer_list<std::pair<const std::string, JsonValue>> init)
            : std::variant<std::string, JsonObject, int, float, JsonArray>(
                JsonObject(init))
        {
        }

        std::string serialize() const;

        JsonValue& operator[](size_t index);

        JsonValue& operator[](const std::string& key);

        void printType() const;

        static JsonValue parseJson(const std::string& json)
        {
            size_t pos = 0; // Keep track of the current position in the string
            return parseValue(json, pos);
        }
    };

    std::string JsonValue::serialize() const
    {
        std::ostringstream os;
        if (std::holds_alternative<std::string>(*this)) {
            os << "\"" << std::get<std::string>(*this) << "\"";
        } else if (std::holds_alternative<JsonObject>(*this)) {
            const auto& obj = std::get<JsonObject>(*this);
            os << "{";
            for (auto iter = obj.begin(); iter != obj.end();) {
                os << "\"" << iter->first << "\": " << iter->second.serialize();
                if (++iter != obj.end()) {
                    os << ", ";
                }
            }
            os << "}";
        } else if (std::holds_alternative<int>(*this)) {
            os << std::get<int>(*this);
        } else if (std::holds_alternative<float>(*this)) {
            os << std::get<float>(*this);
        } else if (std::holds_alternative<JsonArray>(*this)) {
            const auto& array = std::get<JsonArray>(*this);
            os << "[";
            for (auto iter = array.begin(); iter != array.end();) {
                os << iter->serialize();
                if (++iter != array.end()) {
                    os << ", ";
                }
            }
            os << "]";
        }
        return os.str();
    }

    JsonValue& JsonValue::operator[](size_t index)
    {
        if (std::holds_alternative<JsonArray>(*this)) {
            auto& array = std::get<JsonArray>(*this);
            if (index >= array.size()) {
                throw std::out_of_range(
                    "Index out of range"); // Throw an exception if index is out of range
                // array.resize(index + 1); //Maybe resize the array to accommodate the
                // new index
            }
            return array[index];
        }

        throw std::runtime_error("JsonValue is not an array");
    }

    JsonValue& JsonValue::operator[](const std::string& key)
    {
        if (std::holds_alternative<JsonObject>(*this)) {
            return std::get<JsonObject>(*this)[key];
        }

        throw std::runtime_error("JsonValue is not an object");
    }
    void JsonValue::printType() const
    {
        if (std::holds_alternative<std::string>(*this)) {
            std::cout << "Type: string" << std::endl;
        } else if (std::holds_alternative<JsonObject>(*this)) {
            std::cout << "Type: JsonObject" << std::endl;
        } else if (std::holds_alternative<int>(*this)) {
            std::cout << "Type: int" << std::endl;
        } else if (std::holds_alternative<float>(*this)) {
            std::cout << "Type: float" << std::endl;
        } else if (std::holds_alternative<JsonArray>(*this)) {
            std::cout << "Type: JsonArray" << std::endl;
        } else {
            std::cout << "Unknown Type" << std::endl;
        }
    }

    void skipWhitespace(const std::string& json, size_t& pos)
    {
        while (pos < json.size() && std::isspace(json[pos])) {
            ++pos;
        }
    }

    std::string extractString(const std::string& json, size_t& pos)
    {
        if (json[pos] != '"')
            throw std::runtime_error("JSON parse error: Expected '\"'");
        ++pos; // Skip the opening quotation mark

        std::string result;
        while (pos < json.size() && json[pos] != '"') {
            result += json[pos++];
        }

        if (pos == json.size())
            throw std::runtime_error("JSON parse error: Unexpected end of string");
        ++pos; // Skip the closing quotation mark

        return result;
    }

    JsonObject parseObject(const std::string& json, size_t& pos)
    {
        JsonObject obj;
        ++pos; // Skip the opening '{'

        skipWhitespace(json, pos);
        while (json[pos] != '}') {
            std::string key = extractString(json, pos);
            skipWhitespace(json, pos);

            if (json[pos] != ':')
                throw std::runtime_error("JSON parse error: Expected ':'");
            ++pos; // Skip the colon
            skipWhitespace(json, pos);

            obj[key] = parseValue(json, pos);

            skipWhitespace(json, pos);
            if (json[pos] == ',') {
                ++pos; // Skip comma and continue parsing next key-value pair
                skipWhitespace(json, pos);
            }
        }
        ++pos; // Skip the closing '}'

        return obj;
    }

    JsonArray parseArray(const std::string& json, size_t& pos)
    {
        JsonArray array;
        ++pos; // Skip the opening '['

        skipWhitespace(json, pos);
        while (json[pos] != ']') {
            array.push_back(parseValue(json, pos));

            skipWhitespace(json, pos);
            if (json[pos] == ',') {
                ++pos; // Skip comma and continue parsing next element
                skipWhitespace(json, pos);
            }
        }
        ++pos; // Skip the closing ']'

        return array;
    }

    JsonValue parseNumber(const std::string& json, size_t& pos)
    {
        size_t startPos = pos;
        while (pos < json.size() && (std::isdigit(json[pos]) || json[pos] == '.' || json[pos] == '-' || json[pos] == '+')) {
            ++pos;
        }

        std::string_view numberStr(json.data() + startPos, pos - startPos);
        if (numberStr.find('.') != std::string_view::npos) { // Float
            float value;
            auto [ptr, ec] = std::from_chars(
                numberStr.data(), numberStr.data() + numberStr.size(), value);
            if (ec == std::errc()) {
                return JsonValue(value);
            }
        } else { // Integer
            int value;
            auto [ptr, ec] = std::from_chars(
                numberStr.data(), numberStr.data() + numberStr.size(), value);
            if (ec == std::errc()) {
                return JsonValue(value);
            }
        }

        throw std::runtime_error("JSON parse error: Invalid number");
    }

    JsonValue parseValue(const std::string& json, size_t& pos)
    {
        skipWhitespace(json, pos);
        char valueStart = json[pos];
        if (valueStart == '"') {
            return JsonValue(extractString(json, pos));
        } else if (valueStart == '{') {
            return JsonValue(parseObject(json, pos));
        } else if (valueStart == '[') {
            return JsonValue(parseArray(json, pos));
        } else if (std::isdigit(valueStart) || valueStart == '-' || valueStart == '+') {
            return parseNumber(json, pos);
        } else {
            throw std::runtime_error("JSON parse error: Unexpected value");
        }
    }

} // namespace json
} // namespace Utility
