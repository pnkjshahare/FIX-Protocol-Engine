#include "message_validator.h"
#include "body_length.h"
#include "checksum.h"
#include "tags.h"

#include <vector>

bool MessageValidator::validate(
    const std::string &rawMessage,
    const FixMessage &msg,
    std::string &error)
{
    //--------------------------------------------------
    // Validate BodyLength
    //--------------------------------------------------
    int receivedBodyLength =
        std::stoi(msg.getTag(FIXTags::BodyLength));

    int calculatedBodyLength =
        BodyLength::calculate(rawMessage);

    if (receivedBodyLength != calculatedBodyLength)
    {
        error = "Invalid BodyLength";
        return false;
    }

    //--------------------------------------------------
    // Validate CheckSum
    //--------------------------------------------------
    int receivedChecksum =
        std::stoi(msg.getTag(FIXTags::CheckSum));

    int calculatedChecksum =
        calculateChecksumForValidation(rawMessage);

    if (receivedChecksum != calculatedChecksum)
    {
        error = "Invalid CheckSum";
        return false;
    }

    //--------------------------------------------------
    // Read MsgType
    //--------------------------------------------------
    std::string msgType =
        msg.getTag(FIXTags::MsgType);

    //--------------------------------------------------
    // Required Tags : Logon
    //--------------------------------------------------
    std::vector<int> logonTags =
        {
            FIXTags::BeginString,
            FIXTags::BodyLength,
            FIXTags::MsgType,
            FIXTags::SenderCompID,
            FIXTags::TargetCompID,
            FIXTags::MsgSeqNum,
            FIXTags::SendingTime,
            FIXTags::EncryptMethod,
            FIXTags::HeartBtInt,
            FIXTags::CheckSum};

    //--------------------------------------------------
    // Required Tags : Heartbeat
    //--------------------------------------------------
    std::vector<int> heartbeatTags =
        {
            FIXTags::BeginString,
            FIXTags::BodyLength,
            FIXTags::MsgType,
            FIXTags::SenderCompID,
            FIXTags::TargetCompID,
            FIXTags::MsgSeqNum,
            FIXTags::SendingTime,
            FIXTags::CheckSum};

    //--------------------------------------------------
    // Required Tags : Logout
    //--------------------------------------------------
    std::vector<int> logoutTags =
        {
            FIXTags::BeginString,
            FIXTags::BodyLength,
            FIXTags::MsgType,
            FIXTags::SenderCompID,
            FIXTags::TargetCompID,
            FIXTags::MsgSeqNum,
            FIXTags::SendingTime,
            FIXTags::CheckSum};

    //--------------------------------------------------
    // Required Tags : New Order
    //--------------------------------------------------
    std::vector<int> orderTags =
        {
            FIXTags::BeginString,
            FIXTags::BodyLength,
            FIXTags::MsgType,
            FIXTags::SenderCompID,
            FIXTags::TargetCompID,
            FIXTags::MsgSeqNum,
            FIXTags::SendingTime,
            FIXTags::ClOrdID,
            FIXTags::Symbol,
            FIXTags::Side,
            FIXTags::OrderQty,
            FIXTags::OrdType,
            FIXTags::Price,
            FIXTags::TimeInForce,
            FIXTags::CheckSum};

    //--------------------------------------------------
    // Required Tags : Execution Report
    //--------------------------------------------------
    std::vector<int> executionReportTags =
        {
            FIXTags::BeginString,
            FIXTags::BodyLength,
            FIXTags::MsgType,
            FIXTags::SenderCompID,
            FIXTags::TargetCompID,
            FIXTags::MsgSeqNum,
            FIXTags::SendingTime,
            FIXTags::OrderID,
            FIXTags::ExecID,
            FIXTags::ClOrdID,
            FIXTags::Symbol,
            FIXTags::Side,
            FIXTags::OrderQty,
            FIXTags::Price,
            FIXTags::ExecType,
            FIXTags::OrdStatus,
            FIXTags::CheckSum};

    //--------------------------------------------------
    // Select Required Tag Set
    //--------------------------------------------------
    const std::vector<int> *requiredTags = nullptr;

    if (msgType == "A")
    {
        requiredTags = &logonTags;
    }
    else if (msgType == "0")
    {
        requiredTags = &heartbeatTags;
    }
    else if (msgType == "5")
    {
        requiredTags = &logoutTags;
    }
    else if (msgType == "D")
    {
        requiredTags = &orderTags;
    }
    else if (msgType == "8")
    {
        requiredTags = &executionReportTags;
    }
    else
    {
        error = "Unsupported MsgType";
        return false;
    }

    //--------------------------------------------------
    // Validate Required Tags
    //--------------------------------------------------
    for (int tag : *requiredTags)
    {
        if (!msg.hasTag(tag))
        {
            error =
                "Missing Required Tag : " +
                std::to_string(tag);

            return false;
        }
    }

    //--------------------------------------------------
    // Logon Validation
    //--------------------------------------------------
    if (msgType == "A")
    {
        if (msg.getTag(FIXTags::EncryptMethod) != "0")
        {
            error = "Invalid EncryptMethod";
            return false;
        }

        if (std::stoi(msg.getTag(FIXTags::HeartBtInt)) <= 0)
        {
            error = "Invalid HeartBtInt";
            return false;
        }
    }

    //--------------------------------------------------
    // Heartbeat Validation
    //--------------------------------------------------
    if (msgType == "0")
    {
        // No additional validation
    }

    //--------------------------------------------------
    // Logout Validation
    //--------------------------------------------------
    if (msgType == "5")
    {
        // No additional validation
    }

    //--------------------------------------------------
    // New Order Validation
    //--------------------------------------------------
    if (msgType == "D")
    {
        if (std::stoi(msg.getTag(FIXTags::OrderQty)) <= 0)
        {
            error = "Invalid Quantity";
            return false;
        }

        if (std::stod(msg.getTag(FIXTags::Price)) <= 0)
        {
            error = "Invalid Price";
            return false;
        }

        if (msg.getTag(FIXTags::Side) != "1" &&
            msg.getTag(FIXTags::Side) != "2")
        {
            error = "Invalid Side";
            return false;
        }
    }

    //--------------------------------------------------
    // Execution Report Validation
    //--------------------------------------------------
    if (msgType == "8")
    {
        if (msg.getTag(FIXTags::ExecType) != "0")
        {
            error = "Invalid ExecType";
            return false;
        }

        if (msg.getTag(FIXTags::OrdStatus) != "0")
        {
            error = "Invalid OrdStatus";
            return false;
        }

        if (std::stoi(msg.getTag(FIXTags::OrderQty)) <= 0)
        {
            error = "Invalid Quantity";
            return false;
        }

        if (std::stod(msg.getTag(FIXTags::Price)) <= 0)
        {
            error = "Invalid Price";
            return false;
        }

        if (msg.getTag(FIXTags::Side) != "1" &&
            msg.getTag(FIXTags::Side) != "2")
        {
            error = "Invalid Side";
            return false;
        }
    }

    return true;
}