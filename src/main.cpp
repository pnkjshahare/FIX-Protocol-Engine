#include <iostream>
#include "message.h"
#include "encoder.h"
#include "validator.h"
#include "parser.h"
using namespace std;

int main()
{
    int choice;
    Order order;

    cout << "========== Stock Trading ==========\n";
    cout << "1. Buy\n";
    cout << "2. Sell\n";
    cout << "Enter your choice: ";
    cin >> choice;

    switch (choice)
    {
    case 1:
        order.side = '1'; // FIX Side = Buy
        break;

    case 2:
        order.side = '2'; // FIX Side = Sell
        break;

    default:
        cout << "Invalid Choice\n";
        return 0;
    }

    cout << "\nEnter Stock Name : ";
    cin >> order.symbol;

    cout << "Enter Quantity   : ";
    cin >> order.quantity;

    cout << "Enter Price      : ";
    cin >> order.price;

    order.clOrdID = "ORD000001";
    std::string errorMessage;
    if (!Validator::validate(order, errorMessage))
    {
        cout << "Validation Error: " << errorMessage << endl;
        return 0;
    }
    Encoder encoder;

    string fixMessage = encoder.encode(order);

    cout << "\nGenerated FIX Message\n";
    cout << fixMessage << endl;

    Parser parser;

    auto fields = parser.parse(fixMessage);
    cout << "\nParsed Fields\n";

    for (const auto &field : fields)
    {
        std::cout << field.first
                  << " -> "
                  << field.second
                  << std::endl;
    }

    return 0;
}