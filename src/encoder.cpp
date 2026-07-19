#include "encoder.h"
#include "tags.h"
#include "utils.h"
#include "checksum.h"

#include <iomanip>

using namespace std;

// =====================================================
// Common Helper
// =====================================================
string Encoder::buildMessage(const stringstream &body)
{
    string soh = "|";

    // Calculate BodyLength
    string bodyStr = body.str();
    int bodyLength = bodyStr.size();

    // Build FIX Header
    stringstream fix;

    appendTag(FIXTags::BeginString,
              "FIX.4.4",
              fix,
              soh);

    appendTag(FIXTags::BodyLength,
              bodyLength,
              fix,
              soh);

    // Append Body
    fix << bodyStr;

    // Calculate CheckSum
    int checksum = calculateChecksum(fix.str());

    stringstream checksumStream;
    checksumStream << setw(3)
                   << setfill('0')
                   << checksum;

    // Append CheckSum
    appendTag(FIXTags::CheckSum,
              checksumStream.str(),
              fix,
              soh);

    return fix.str();
}

// =====================================================
// New Order Single (35=D)
// =====================================================
string Encoder::encode(const Order &order, Session &session)
{
    string soh = "|";

    stringstream body;

    appendTag(FIXTags::MsgType, "D", body, soh);
    appendTag(FIXTags::SenderCompID, "CLIENT1", body, soh);
    appendTag(FIXTags::TargetCompID, "EXCHANGE", body, soh);

    appendTag(FIXTags::MsgSeqNum,
              session.getNextOutgoingSeqNum(),
              body,
              soh);

    appendTag(FIXTags::SendingTime,
              getCurrentUTCTime(),
              body,
              soh);

    appendTag(FIXTags::ClOrdID,
              order.clOrdID,
              body,
              soh);

    appendTag(FIXTags::Symbol,
              order.symbol,
              body,
              soh);

    appendTag(FIXTags::Side,
              order.side,
              body,
              soh);

    appendTag(FIXTags::OrderQty,
              order.quantity,
              body,
              soh);

    appendTag(FIXTags::OrdType,
              2,
              body,
              soh);

    appendTag(FIXTags::Price,
              order.price,
              body,
              soh);

    appendTag(FIXTags::TimeInForce,
              0,
              body,
              soh);

    return buildMessage(body);
}

// =====================================================
// Logon (35=A)
// =====================================================
string Encoder::encodeLogon(Session &session)
{
    string soh = "|";

    stringstream body;

    appendTag(FIXTags::MsgType, "A", body, soh);

    appendTag(FIXTags::SenderCompID,
              "CLIENT1",
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              "EXCHANGE",
              body,
              soh);

    appendTag(FIXTags::MsgSeqNum,
              session.getNextOutgoingSeqNum(),
              body,
              soh);

    appendTag(FIXTags::SendingTime,
              getCurrentUTCTime(),
              body,
              soh);

    appendTag(FIXTags::EncryptMethod,
              0,
              body,
              soh);

    appendTag(FIXTags::HeartBtInt,
              30,
              body,
              soh);

    return buildMessage(body);
}

// =====================================================
// Heartbeat (35=0)
// =====================================================
string Encoder::encodeHeartbeat(Session &session)
{
    string soh = "|";

    stringstream body;

    appendTag(FIXTags::MsgType,
              "0",
              body,
              soh);

    appendTag(FIXTags::SenderCompID,
              "CLIENT1",
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              "EXCHANGE",
              body,
              soh);

    appendTag(FIXTags::MsgSeqNum,
              session.getNextOutgoingSeqNum(),
              body,
              soh);

    appendTag(FIXTags::SendingTime,
              getCurrentUTCTime(),
              body,
              soh);

    return buildMessage(body);
}

// =====================================================
// Logout (35=5)
// =====================================================
string Encoder::encodeLogout(Session &session)
{
    string soh = "|";

    stringstream body;

    appendTag(FIXTags::MsgType,
              "5",
              body,
              soh);

    appendTag(FIXTags::SenderCompID,
              "CLIENT1",
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              "EXCHANGE",
              body,
              soh);

    appendTag(FIXTags::MsgSeqNum,
              session.getNextOutgoingSeqNum(),
              body,
              soh);

    appendTag(FIXTags::SendingTime,
              getCurrentUTCTime(),
              body,
              soh);

    return buildMessage(body);
}