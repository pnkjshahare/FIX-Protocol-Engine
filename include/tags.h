#ifndef TAGS_H
#define TAGS_H
namespace FIXTags
{
    constexpr int BeginString = 8;
    constexpr int BodyLength = 9;
    constexpr int MsgType = 35;
    constexpr int SenderCompID = 49;
    constexpr int TargetCompID = 56;
    constexpr int MsgSeqNum = 34;
    constexpr int SendingTime = 52;

    // Order Fields
    constexpr int ClOrdID = 11;
    constexpr int HandlInst = 21;
    constexpr int Symbol = 55;
    constexpr int Side = 54;
    constexpr int OrderQty = 38;
    constexpr int OrdType = 40;
    constexpr int Price = 44;
    constexpr int TimeInForce = 59;

    // Trailer
    constexpr int CheckSum = 10;

    constexpr int EncryptMethod = 98;
    constexpr int HeartBtInt = 108;
}
#endif // TAGS_H