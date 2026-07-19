#include "session.h"

Session::Session()
{
    loggedOn = false;
    outgoingSeqNum = 1;
    incomingSeqNum = 1;
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

int Session::getNextOutgoingSeqNum()
{
    return outgoingSeqNum++;
}

void Session::incrementIncomingSeqNum()
{
    incomingSeqNum++;
}

int Session::getIncomingSeqNum() const
{
    return incomingSeqNum;
}