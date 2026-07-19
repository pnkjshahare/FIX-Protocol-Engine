#ifndef MESSAGE_VALIDATOR_H
#define MESSAGE_VALIDATOR_H

#include <string>
#include "fix_message.h"

class MessageValidator
{
public:
    static bool validate(
        const std::string &rawMessage,
        const FixMessage &msg,
        std::string &error);
};

#endif