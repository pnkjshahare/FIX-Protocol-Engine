#ifndef SESSION_H
#define SESSION_H

#include <string>

class Session
{
private:
    bool loggedOn;

    int outgoingSeqNum;

    int expectedIncomingSeqNum;

    int heartBtInt;

public:
    Session();

    bool isLoggedOn() const;

    void logon();

    void logout();

    void reset();

    int getNextOutgoingSeqNum();

    bool validateIncomingSeqNum(int receivedSeqNum, std::string &error);

    int getIncomingSeqNum() const;

    int getHeartBtInt() const;

    void setHeartBtInt(int seconds);
};

#endif
