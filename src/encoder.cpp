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
    // string soh = "|";
    string soh = "\x01";

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
    // string soh = "|";
    string soh = "\x01";

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
    return encodeLogon(session, "CLIENT1", "EXCHANGE");
}

string Encoder::encodeLogon(Session &session,
                            const string &senderCompID,
                            const string &targetCompID)
{
    // string soh = "|";
    string soh = "\x01";

    stringstream body;

    appendTag(FIXTags::MsgType, "A", body, soh);

    appendTag(FIXTags::SenderCompID,
              senderCompID,
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              targetCompID,
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
    return encodeHeartbeat(session, "CLIENT1", "EXCHANGE");
}

string Encoder::encodeHeartbeat(Session &session,
                                const string &senderCompID,
                                const string &targetCompID)
{
    return encodeHeartbeat(session, senderCompID, targetCompID, "");
}

string Encoder::encodeHeartbeat(Session &session,
                                const string &senderCompID,
                                const string &targetCompID,
                                const string &testReqID)
{
    // string soh = "|";
    string soh = "\x01";

    stringstream body;

    appendTag(FIXTags::MsgType,
              "0",
              body,
              soh);

    appendTag(FIXTags::SenderCompID,
              senderCompID,
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              targetCompID,
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

    if (!testReqID.empty())
    {
        appendTag(FIXTags::TestReqID,
                  testReqID,
                  body,
                  soh);
    }

    return buildMessage(body);
}

string Encoder::encodeCancelOrder(const CancelOrderRequest &request,
                                  Session &session)
{
    string soh = "\x01";

    stringstream body;

    appendTag(FIXTags::MsgType, "F", body, soh);
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

    appendTag(FIXTags::OrigClOrdID,
              request.origClOrdID,
              body,
              soh);

    appendTag(FIXTags::ClOrdID,
              request.clOrdID,
              body,
              soh);

    appendTag(FIXTags::Symbol,
              request.symbol,
              body,
              soh);

    appendTag(FIXTags::Side,
              request.side,
              body,
              soh);

    appendTag(FIXTags::OrderQty,
              request.quantity,
              body,
              soh);

    return buildMessage(body);
}

string Encoder::encodeModifyOrder(const ModifyOrderRequest &request,
                                  Session &session)
{
    string soh = "\x01";

    stringstream body;

    appendTag(FIXTags::MsgType, "G", body, soh);
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

    appendTag(FIXTags::OrigClOrdID,
              request.origClOrdID,
              body,
              soh);

    appendTag(FIXTags::ClOrdID,
              request.clOrdID,
              body,
              soh);

    appendTag(FIXTags::Symbol,
              request.symbol,
              body,
              soh);

    appendTag(FIXTags::Side,
              request.side,
              body,
              soh);

    appendTag(FIXTags::OrderQty,
              request.quantity,
              body,
              soh);

    appendTag(FIXTags::OrdType,
              2,
              body,
              soh);

    appendTag(FIXTags::Price,
              request.price,
              body,
              soh);

    appendTag(FIXTags::TimeInForce,
              0,
              body,
              soh);

    return buildMessage(body);
}

string Encoder::encodeTestRequest(Session &session,
                                  const string &senderCompID,
                                  const string &targetCompID,
                                  const string &testReqID)
{
    string soh = "\x01";

    stringstream body;

    appendTag(FIXTags::MsgType,
              "1",
              body,
              soh);

    appendTag(FIXTags::SenderCompID,
              senderCompID,
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              targetCompID,
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

    appendTag(FIXTags::TestReqID,
              testReqID,
              body,
              soh);

    return buildMessage(body);
}

// =====================================================
// Logout (35=5)
// =====================================================
string Encoder::encodeLogout(Session &session)
{
    return encodeLogout(session, "CLIENT1", "EXCHANGE");
}

string Encoder::encodeLogout(Session &session,
                             const string &senderCompID,
                             const string &targetCompID)
{
    // string soh = "|";
    string soh = "\x01";

    stringstream body;

    appendTag(FIXTags::MsgType,
              "5",
              body,
              soh);

    appendTag(FIXTags::SenderCompID,
              senderCompID,
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              targetCompID,
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

string Encoder::encodeExecutionReport(
    const ExecutionReport &report,
    Session &session)
{
    string soh = "\x01";

    stringstream body;

    appendTag(FIXTags::MsgType,
              "8",
              body,
              soh);

    appendTag(FIXTags::SenderCompID,
              "EXCHANGE",
              body,
              soh);

    appendTag(FIXTags::TargetCompID,
              "CLIENT1",
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

    appendTag(FIXTags::OrderID,
              report.orderID,
              body,
              soh);

    appendTag(FIXTags::ExecID,
              report.execID,
              body,
              soh);

    appendTag(FIXTags::ClOrdID,
              report.clOrdID,
              body,
              soh);

    if (!report.origClOrdID.empty())
    {
        appendTag(FIXTags::OrigClOrdID,
                  report.origClOrdID,
                  body,
                  soh);
    }

    appendTag(FIXTags::Symbol,
              report.symbol,
              body,
              soh);

    appendTag(FIXTags::Side,
              report.side,
              body,
              soh);

    appendTag(FIXTags::OrderQty,
              report.quantity,
              body,
              soh);

    appendTag(FIXTags::Price,
              report.price,
              body,
              soh);

    appendTag(FIXTags::ExecType,
              report.execType,
              body,
              soh);

    appendTag(FIXTags::OrdStatus,
              report.ordStatus,
              body,
              soh);

    return buildMessage(body);
}
