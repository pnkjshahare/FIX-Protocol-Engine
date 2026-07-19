#ifndef VALIDATOR_H
#define VALIDATOR_H
#include <string>
#include "message.h"

class Validator
{
public:
    static bool validate(const Order &order, std::string &errorMessage);
};
#endif // VALIDATOR_H