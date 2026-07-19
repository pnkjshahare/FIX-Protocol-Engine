#ifndef SESSION_H
#define SESSION_H

class Session
{
private:
    bool loggedOn;

    int outgoingSeqNum;

    int incomingSeqNum;

public:
    Session();

    bool isLoggedOn() const;

    void logon();

    void logout();

    int getNextOutgoingSeqNum();

    void incrementIncomingSeqNum();

    int getIncomingSeqNum() const;
};

#endif