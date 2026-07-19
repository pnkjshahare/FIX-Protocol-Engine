#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <unordered_map>

class Parser
{
public:
    std::unordered_map<int, std::string>
    parse(const std::string &message);
};

#endif