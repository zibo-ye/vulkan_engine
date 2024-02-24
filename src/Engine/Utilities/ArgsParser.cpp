
#include "ArgsParser.hpp"

using namespace Utility;

std::optional<std::vector<std::string>> ArgsParser::GetArg(const std::string& argName) const
{
    auto it = m_Args.find(argName);
    if (it != m_Args.end()) {
        return it->second;
    }
    return std::nullopt;
}

void ArgsParser::PrintArgs()
{
    std::cout << "ArgsParser parsed the following parameters: " << std::endl;
    for (const auto& [key, values] : m_Args) {
        std::cout << "Key:" << key << "\tValue:";
        for (const auto& value : values) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

ArgsParser::ArgsParser(int argc, const char** argv)
{
#ifdef VERBOSE
    std::cout << "ArgsParser parsing the following parameters: " << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
#endif
    std::string currentKey;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg[0] == '-') { // Key&Value
            if (arg[1] == '-') // Key only
            {
                currentKey = arg.substr(2); // Remove --
                m_Args[currentKey] = std::vector<std::string>();
            } else {
                std::replace(arg.begin(), arg.end(), '\xa0', '\x20'); // Replace non-breaking spaces with regular spaces
                size_t firstSpacePos = arg.find('\x20'); // ascii space
                if (firstSpacePos != std::string::npos) {
                    // Extract the key using the position of the first space
                    currentKey = arg.substr(1, firstSpacePos - 1); // Remove -
                    m_Args[currentKey].push_back(arg.substr(firstSpacePos + 1));
                } else {
                    currentKey = arg.substr(1); // Remove -
                    m_Args[currentKey] = std::vector<std::string>();
                }
            }
        } else if (!currentKey.empty()) { // Value
            std::string value = arg;

            if (value.front() == '"' && value.back() == '"') {
                // Remove quotes from values enclosed in quotes
                value = value.substr(1, value.length() - 2);
            }
            m_Args[currentKey].push_back(value);
        }
    }

    if (GetArg("verbose")) {
        PrintArgs();
    } else {
#ifdef VERBOSE
        PrintArgs();
#endif
    }
}
