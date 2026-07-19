#include "encoder.h"
#include "tags.h"
#include "utils.h"
#include "checksum.h"
#include <iomanip>
using namespace std;

string Encoder::encode(const Order &order)
{
    string soh = "|";

    // Build only the message body
    stringstream body;

    appendTag(FIXTags::MsgType, "D", body, soh);
    appendTag(FIXTags::SenderCompID, "CLIENT1", body, soh);
    appendTag(FIXTags::TargetCompID, "EXCHANGE", body, soh);
    appendTag(FIXTags::MsgSeqNum, 1, body, soh);
    appendTag(FIXTags::SendingTime, getCurrentUTCTime(), body, soh);

    appendTag(FIXTags::ClOrdID, order.clOrdID, body, soh);
    appendTag(FIXTags::Symbol, order.symbol, body, soh);
    appendTag(FIXTags::Side, order.side, body, soh);
    appendTag(FIXTags::OrderQty, order.quantity, body, soh);
    appendTag(FIXTags::OrdType, 2, body, soh);
    appendTag(FIXTags::Price, order.price, body, soh);
    appendTag(FIXTags::TimeInForce, 0, body, soh);
    // appendTag(FIXTags::CheckSum, calculateChecksum(body.str()), body, soh);

    // Calculate BodyLength
    // Calculate BodyLength
    string bodyStr = body.str();
    int bodyLength = bodyStr.size();

    // Build final FIX message
    stringstream fix;

    appendTag(FIXTags::BeginString, "FIX.4.4", fix, soh);
    appendTag(FIXTags::BodyLength, bodyLength, fix, soh);

    fix << bodyStr;

    // Calculate checksum on the complete message (without Tag 10)
    string fixMessage = fix.str();
    int checksum = calculateChecksum(fixMessage);

    // Format checksum as 3 digits
    stringstream checksumStream;
    checksumStream << setw(3)
                   << setfill('0')
                   << checksum;

    // Append Tag 10 once
    appendTag(FIXTags::CheckSum, checksumStream.str(), fix, soh);

    return fix.str();
}