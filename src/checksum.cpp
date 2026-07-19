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