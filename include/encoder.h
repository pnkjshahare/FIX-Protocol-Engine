#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <sstream>

#include "message.h"
#include "session.h"
#include "execution_report.h"

class Encoder
{
public:
    // New Order Single (35=D)
    std::string encode(const Order &order, Session &session);

    std::string encodeCancelOrder(const CancelOrderRequest &request,
                                  Session &session);

    std::string encodeModifyOrder(const ModifyOrderRequest &request,
                                  Session &session);

    // Logon (35=A)
    std::string encodeLogon(Session &session);

    std::string encodeLogon(Session &session,
                            const std::string &senderCompID,
                            const std::string &targetCompID);

    // Heartbeat (35=0)
    std::string encodeHeartbeat(Session &session);

    std::string encodeHeartbeat(Session &session,
                                const std::string &senderCompID,
                                const std::string &targetCompID);

    std::string encodeHeartbeat(Session &session,
                                const std::string &senderCompID,
                                const std::string &targetCompID,
                                const std::string &testReqID);

    std::string encodeTestRequest(Session &session,
                                  const std::string &senderCompID,
                                  const std::string &targetCompID,
                                  const std::string &testReqID);

    // Logout (35=5)
    std::string encodeLogout(Session &session);

    std::string encodeLogout(Session &session,
                             const std::string &senderCompID,
                             const std::string &targetCompID);

    // Execution Report (35=8)
    std::string encodeExecutionReport(
        const ExecutionReport &report,
        Session &session);

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
