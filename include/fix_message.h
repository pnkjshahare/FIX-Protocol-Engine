#ifndef FIX_MESSAGE_H
#define FIX_MESSAGE_H

#include <unordered_map>
#include <string>

class FixMessage
{
private:
    std::unordered_map<int, std::string> fields;

public:
    void setTag(int tag, const std::string &value);

    bool hasTag(int tag) const;

    std::string getTag(int tag) const;

    void print() const;
};

#endif