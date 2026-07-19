#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <sstream>

#include "message.h"
#include "session.h"

class Encoder
{
public:
    // New Order Single (35=D)
    std::string encode(const Order &order, Session &session);

    // Logon (35=A)
    std::string encodeLogon(Session &session);

    // Heartbeat (35=0)
    std::string encodeHeartbeat(Session &session);

    // Logout (35=5)
    std::string encodeLogout(Session &session);

private:
    // Common helper to build the complete FIX message
    std::string buildMessage(const std::stringstream &body);

    // Generic helper to append a FIX tag
    template <typename T>
    void appendTag(int tag,
                   const T &value,
                   std::stringstream &ss,
                   const std::string &soh)
    {
        ss << tag
           << "="
           << value
           << soh;
    }
};

#endif // ENCODER_H