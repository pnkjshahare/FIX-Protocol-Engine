#include "validator.h"

bool Validator::validate(const Order &order, std::string &errorMessage)
{
    if (order.symbol.empty())
    {
        errorMessage = "Symbol cannot be empty.";
        return false;
    }

    if (order.quantity <= 0)
    {
        errorMessage = "Quantity must be greater than zero.";
        return false;
    }

    if (order.price <= 0.0)
    {
        errorMessage = "Price must be greater than zero.";
        return false;
    }

    if (order.side != '1' && order.side != '2')
    {
        errorMessage = "Side must be '1' (Buy) or '2' (Sell).";
        return false;
    }

    return true;
}