#include "parser.h"
#include "fix_message.h"
#include <sstream>

// std::unordered_map<int, std::string>
FixMessage messageObj;
FixMessage Parser::parse(const std::string &message)
{
    FixMessage messageObj;

    std::stringstream ss(message);
    std::string token;

    while (std::getline(ss, token, '|'))
    {
        size_t pos = token.find('=');

        if (pos == std::string::npos)
            continue;

        int tag = std::stoi(token.substr(0, pos));
        std::string value = token.substr(pos + 1);

        messageObj.setTag(tag, value);
    }

    return messageObj;
}