#include "checksum.h"

int calculateChecksum(const std::string &message)
{
    int sum = 0;

    for (unsigned char ch : message)
    {
        sum += ch;
    }

    return sum % 256;
}

int calculateChecksumForValidation(const std::string &message)
{
    size_t checksumPos = message.find("10=");

    if (checksumPos == std::string::npos)
        return -1;

    int sum = 0;

    for (size_t i = 0; i < checksumPos; i++)
    {
        sum += static_cast<unsigned char>(message[i]);
    }

    return sum % 256;
}