#include "order_book.h"
#include "tags.h"

#include <iomanip>
#include <iostream>
#include <sstream>

OrderBook::OrderBook()
{
    nextOrderNumber = 1;
}

StoredOrder OrderBook::addOrder(const FixMessage &message)
{
    std::stringstream orderID;
    orderID << "ORDER"
            << std::setw(4)
            << std::setfill('0')
            << nextOrderNumber++;

    StoredOrder order;

    order.orderID = orderID.str();
    order.clOrdID = message.getTag(FIXTags::ClOrdID);
    order.symbol = message.getTag(FIXTags::Symbol);
    order.side = message.getTag(FIXTags::Side)[0];
    order.quantity = std::stoi(message.getTag(FIXTags::OrderQty));
    order.price = std::stod(message.getTag(FIXTags::Price));
    order.ordStatus = '0';

    orders[order.clOrdID] = order;

    return order;
}

bool OrderBook::cancelOrder(const std::string &origClOrdID,
                            const std::string &cancelClOrdID,
                            StoredOrder &order)
{
    auto it = orders.find(origClOrdID);

    if (it == orders.end() || it->second.ordStatus == '4')
    {
        return false;
    }

    it->second.clOrdID = cancelClOrdID;
    it->second.ordStatus = '4';
    order = it->second;

    orders.erase(it);
    orders[order.clOrdID] = order;

    return true;
}

bool OrderBook::modifyOrder(const FixMessage &message,
                            StoredOrder &order)
{
    std::string origClOrdID =
        message.getTag(FIXTags::OrigClOrdID);

    auto it = orders.find(origClOrdID);

    if (it == orders.end() || it->second.ordStatus == '4')
    {
        return false;
    }

    it->second.clOrdID = message.getTag(FIXTags::ClOrdID);
    it->second.quantity = std::stoi(message.getTag(FIXTags::OrderQty));
    it->second.price = std::stod(message.getTag(FIXTags::Price));
    it->second.ordStatus = '0';
    order = it->second;

    orders.erase(it);
    orders[order.clOrdID] = order;

    return true;
}

void OrderBook::print() const
{
    std::cout << "\n====================================\n";
    std::cout << " Current Order Book\n";
    std::cout << "====================================\n";

    if (orders.empty())
    {
        std::cout << "No orders\n";
        return;
    }

    for (const auto &entry : orders)
    {
        const StoredOrder &order = entry.second;

        std::cout << "OrderID=" << order.orderID
                  << " ClOrdID=" << order.clOrdID
                  << " Symbol=" << order.symbol
                  << " Side=" << order.side
                  << " Qty=" << order.quantity
                  << " Price=" << order.price
                  << " Status=" << order.ordStatus
                  << std::endl;
    }
}
