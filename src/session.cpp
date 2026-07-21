#include "session.h"

Session::Session()
{
    reset();
}

bool Session::isLoggedOn() const
{
    return loggedOn;
}

void Session::logon()
{
    loggedOn = true;
}

void Session::logout()
{
    loggedOn = false;
}

void Session::reset()
{
    loggedOn = false;
    outgoingSeqNum = 1;
    expectedIncomingSeqNum = 1;
    heartBtInt = 30;
}

int Session::getNextOutgoingSeqNum()
{
    return outgoingSeqNum++;
}

bool Session::validateIncomingSeqNum(int receivedSeqNum, std::string &error)
{
    if (receivedSeqNum != expectedIncomingSeqNum)
    {
        error =
            "Invalid MsgSeqNum. Expected " +
            std::to_string(expectedIncomingSeqNum) +
            " but received " +
            std::to_string(receivedSeqNum);

        return false;
    }

    expectedIncomingSeqNum++;
    return true;
}

int Session::getIncomingSeqNum() const
{
    return expectedIncomingSeqNum;
}

int Session::getHeartBtInt() const
{
    return heartBtInt;
}

void Session::setHeartBtInt(int seconds)
{
    heartBtInt = seconds;
}
