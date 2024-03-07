
#include "Utility.hpp"

using namespace Utility::json;

std::string JsonValue::serialize(bool beautify /*= false*/, int indentLevel /*= 0*/) const
{
    std::ostringstream os;
    std::string indent = beautify ? std::string(indentLevel * 2, ' ') : "";
    std::string newline = beautify ? "\n" : "";
    std::string separator = beautify ? ",\n" : ", ";
    std::string space = beautify ? " " : "";

    if (std::holds_alternative<std::string>(*this)) {
        os << "\"" << std::get<std::string>(*this) << "\"";
    } else if (std::holds_alternative<JsonObject>(*this)) {
        const auto& obj = std::get<JsonObject>(*this);
        os << "{" << newline;
        for (auto iter = obj.begin(); iter != obj.end();) {
            os << indent << (beautify ? "  " : "") << "\"" << iter->first << "\":" << space << iter->second.serialize(beautify, indentLevel + 1);
            if (++iter != obj.end()) {
                os << separator;
            }
        }
        os << newline << indent << "}";
    } else if (std::holds_alternative<int>(*this)) {
        os << std::get<int>(*this);
    } else if (std::holds_alternative<float>(*this)) {
        os << std::get<float>(*this);
    } else if (std::holds_alternative<JsonArray>(*this)) {
        const auto& array = std::get<JsonArray>(*this);
        os << "[" << newline;
        for (auto iter = array.begin(); iter != array.end();) {
            os << indent << (beautify ? "  " : "") << iter->serialize(beautify, indentLevel + 1);
            if (++iter != array.end()) {
                os << separator;
            }
        }
        os << newline << indent << "]";
    }
    return os.str();
}

const JsonValue& JsonValue::operator[](size_t index) const
{
    if (isArray()) {
        auto& array = std::get<JsonArray>(*this);
        if (index >= array.size()) {
            throw std::out_of_range(
                "Index out of range"); // Throw an exception if index is out of range
            // array.resize(index + 1); //Maybe resize the array to accommodate the new index
        }
        return array[index];
    }

    throw std::runtime_error("JsonValue is not an array");
}

const JsonValue& JsonValue::operator[](const std::string& key) const
{
    if (isObject()) {
        return std::get<JsonObject>(*this).at(key);
    }

    throw std::runtime_error("JsonValue is not an object");
}

void JsonValue::printType() const
{
    if (isString()) {
        std::cout << "Type: string" << std::endl;
    } else if (isObject()) {
        std::cout << "Type: JsonObject" << std::endl;
    } else if (isInt()) {
        std::cout << "Type: int" << std::endl;
    } else if (isFloat()) {
        std::cout << "Type: float" << std::endl;
    } else if (isArray()) {
        std::cout << "Type: JsonArray" << std::endl;
    } else {
        std::cout << "Unknown Type" << std::endl;
    }
}

JsonValue JsonValue::parseJsonFromString(const std::string& jsonStr)
{
    size_t pos = 0; // Keep track of the current position in the string
    return parseValue(jsonStr, pos);
}

JsonValue JsonValue::parseJsonFromFile(const std::string& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Could not open file for reading: " + path);
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    return parseJsonFromString(buffer.str());
}

bool JsonValue::saveJsonToString(const JsonValue& json, std::string& str)
{
    str = json.serialize();
    return true;
}

bool JsonValue::saveJsonToFile(const JsonValue& json, const std::string& path)
{
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Could not open file for writing: " + path);
    }
    std::stringstream buffer;
    buffer << json.serialize();
    return true;
}

// skip whitespace in json, including space (' '), tab ('\t'), newline ('\n'), vertical tab ('\v'), feed ('\f'), and carriage return ('\r').
void JsonValue::skipWhitespace(const std::string& json, size_t& pos)
{
    while (pos < json.size() && std::isspace(json[pos])) {
        ++pos;
    }
}

std::string JsonValue::extractString(const std::string& json, size_t& pos)
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

JsonObject JsonValue::parseObject(const std::string& json, size_t& pos)
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

JsonArray JsonValue::parseArray(const std::string& json, size_t& pos)
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

JsonValue JsonValue::parseNumber(const std::string& json, size_t& pos)
{
    size_t startPos = pos;
    bool hasExponent = false;

    while (pos < json.size()) {
        char ch = json[pos];
        if (std::isdigit(ch) || ch == '.' || ch == '-' || ch == '+') {
            ++pos;
        } else if ((ch == 'e' || ch == 'E') && !hasExponent) { // Exponent can only appear once
            hasExponent = true;
            ++pos;
            if (pos < json.size() && (json[pos] == '+' || json[pos] == '-')) { // Skip the sign of the exponent
                ++pos;
            }
        } else {
            break;
        }
    }

    std::string_view numberStr(json.data() + startPos, pos - startPos);
    if (numberStr.find('.') != std::string_view::npos) { // Float
        try {
            float value = std::stof(std::string(numberStr));
            return JsonValue(value);
        } catch (const std::exception&) {
            throw std::runtime_error("JSON parse error: Invalid float number");
        }
    } else { // Integer
        try {
            int value = std::stoi(std::string(numberStr));
            return JsonValue(value);
        } catch (const std::exception&) {
            throw std::runtime_error("JSON parse error: Invalid int number");
        }
    }
}

JsonValue JsonValue::parseValue(const std::string& json, size_t& pos)
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

bool JsonValue::hasKey(const std::string& key) const
{
    if (isObject()) {
        return std::get<JsonObject>(*this).find(key) != std::get<JsonObject>(*this).end();
    }
    return false;
}

std::optional<Utility::json::JsonValue> JsonValue::getOptionalValue(const std::string& key) const
{
    if (isObject()) {
        auto& obj = std::get<JsonObject>(*this);
        auto iter = obj.find(key);
        if (iter != obj.end()) {
            return iter->second;
        }
    }
    return std::nullopt;
}
