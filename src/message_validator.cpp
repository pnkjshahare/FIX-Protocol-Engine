#include "message_validator.h"
#include "body_length.h"
#include "checksum.h"

#include <vector>

bool MessageValidator::validate(
    const std::string &rawMessage,
    const FixMessage &msg,
    std::string &error)
{
    std::vector<int> requiredTags =
        {
            8,  // BeginString
            9,  // BodyLength
            35, // MsgType
            49, // SenderCompID
            56, // TargetCompID
            34, // MsgSeqNum
            52, // SendingTime
            11, // ClOrdID
            55, // Symbol
            54, // Side
            38, // OrderQty
            40, // OrdType
            44, // Price
            59, // TimeInForce
            10  // CheckSum
        };

    // ----------------------------
    // Check required tags
    // ----------------------------
    for (int tag : requiredTags)
    {
        if (!msg.hasTag(tag))
        {
            error = "Missing Required Tag : " + std::to_string(tag);
            return false;
        }
    }

    // ----------------------------
    // Validate BodyLength (Tag 9)
    // ----------------------------
    int receivedBodyLength = std::stoi(msg.getTag(9));
    int calculatedBodyLength = BodyLength::calculate(rawMessage);

    if (receivedBodyLength != calculatedBodyLength)
    {
        error = "Invalid BodyLength";
        return false;
    }

    // ----------------------------
    // Validate CheckSum (Tag 10)
    // ----------------------------
    int receivedChecksum = std::stoi(msg.getTag(10));
    // int calculatedChecksum = calculateChecksum(rawMessage);
    int calculatedChecksum =
        calculateChecksumForValidation(rawMessage);
    if (receivedChecksum != calculatedChecksum)
    {
        error = "Invalid CheckSum";
        return false;
    }

    // ----------------------------
    // Validate MsgType
    // ----------------------------
    if (msg.getTag(35) != "D")
    {
        error = "Unsupported MsgType";
        return false;
    }

    // ----------------------------
    // Validate Quantity
    // ----------------------------
    if (std::stoi(msg.getTag(38)) <= 0)
    {
        error = "Invalid Quantity";
        return false;
    }

    // ----------------------------
    // Validate Price
    // ----------------------------
    if (std::stod(msg.getTag(44)) <= 0)
    {
        error = "Invalid Price";
        return false;
    }

    // ----------------------------
    // Validate Side
    // ----------------------------
    if (msg.getTag(54) != "1" &&
        msg.getTag(54) != "2")
    {
        error = "Invalid Side";
        return false;
    }

    return true;
}