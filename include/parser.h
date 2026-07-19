#ifndef PARSER_H
#define PARSER_H
#include "fix_message.h"
#include <string>
#include <unordered_map>

class Parser
{
public:
    // std::unordered_map<int, std::string>
    FixMessage parse(const std::string &message);
};

#endif