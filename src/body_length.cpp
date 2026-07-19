#include "body_length.h"

int BodyLength::calculate(const std::string &message)
{
    size_t bodyStart = message.find("35=");
    size_t checksumPos = message.find("10=");

    if (bodyStart == std::string::npos ||
        checksumPos == std::string::npos)
    {
        return -1;
    }

    return checksumPos - bodyStart;
}