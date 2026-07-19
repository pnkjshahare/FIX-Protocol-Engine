#include "fix_message.h"

#include <iostream>

void FixMessage::setTag(int tag, const std::string &value)
{
    fields[tag] = value;
}

bool FixMessage::hasTag(int tag) const
{
    return fields.find(tag) != fields.end();
}

std::string FixMessage::getTag(int tag) const
{
    auto it = fields.find(tag);

    if (it != fields.end())
        return it->second;

    return "";
}

void FixMessage::print() const
{
    for (const auto &field : fields)
    {
        std::cout
            << field.first
            << " -> "
            << field.second
            << std::endl;
    }
}