#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H

#include <string>
#include <unordered_map>

#include "fix_message.h"

struct StoredOrder
{
    std::string orderID;
    std::string clOrdID;
    std::string symbol;
    char side;
    int quantity;
    double price;
    char ordStatus;
};

class OrderBook
{
private:
    std::unordered_map<std::string, StoredOrder> orders;
    int nextOrderNumber;

public:
    OrderBook();

    StoredOrder addOrder(const FixMessage &message);

    bool cancelOrder(const std::string &origClOrdID,
                     const std::string &cancelClOrdID,
                     StoredOrder &order);

    bool modifyOrder(const FixMessage &message,
                     StoredOrder &order);

    void print() const;
};

#endif
