#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <string>

int calculateChecksum(const std::string &message);

int calculateChecksumForValidation(const std::string &message);

#endif