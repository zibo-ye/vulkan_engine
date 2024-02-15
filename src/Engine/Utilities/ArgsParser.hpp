#pragma once

#include "pch.hpp"

#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>


namespace Utility {

// This ArgsParser assumes that there is no naked values, i.e. all values must be associated with a key. Either the key has a value or it doesn't.
// Keys are assumed to be strings that start with a '-' or two dashes '--'.
// Keys and values are assumed to be separated by a space. Space is not allowed in keys and values.
// When there is a space in a value, it should be enclosed in quotes. Example: -key "value with space"
//  -<key> <value> [<value2> ...]    - This is used to pass in key-value pairs.
//  --<name>							- This is used to pass in flags.
class ArgsParser {
public:
    ArgsParser(int argc, const char** argv);

    void PrintArgs();

    std::optional<std::vector<std::string>> GetArg(const std::string& argName) const;

private:
    std::unordered_map<std::string, std::vector<std::string>> m_Args;
};
} // namespace Utility