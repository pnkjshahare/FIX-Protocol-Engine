#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <sstream>
#include "message.h"

class Encoder
{
public:
    std::string encode(const Order &order);

private:
    template <typename T>
    void appendTag(int tag,
                   const T &value,
                   std::stringstream &ss,
                   const std::string &soh)
    {
        ss << tag << "=" << value << soh;
    }
};

#endif // ENCODER_H