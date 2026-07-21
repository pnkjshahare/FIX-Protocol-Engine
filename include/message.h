#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
struct Order
{
    std::string clOrdID;
    std::string symbol;
    int quantity;
    double price;
    char side; // 1 for buy, 2 for sell
};

struct CancelOrderRequest
{
    std::string origClOrdID;
    std::string clOrdID;
    std::string symbol;
    int quantity;
    char side;
};

struct ModifyOrderRequest
{
    std::string origClOrdID;
    std::string clOrdID;
    std::string symbol;
    int quantity;
    double price;
    char side;
};
#endif // MESSAGE_H
