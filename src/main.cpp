#include <iostream>

#include "message.h"
#include "encoder.h"
#include "validator.h"
#include "parser.h"
#include "message_validator.h"
#include "body_length.h"
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
        order.side = '1'; // Buy
        break;

    case 2:
        order.side = '2'; // Sell
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

    // Validate Order
    string errorMessage;

    if (!Validator::validate(order, errorMessage))
    {
        cout << "\nValidation Error: " << errorMessage << endl;
        return 0;
    }

    // Encode FIX Message
    Encoder encoder;
    string fixMessage = encoder.encode(order);
    cout << "\nGenerated FIX Message\n";
    cout << fixMessage << endl;

    // Parse FIX Message
    Parser parser;
    FixMessage msg = parser.parse(fixMessage);

    cout << "\nParsed Fields\n";
    msg.print();

    // Validate Parsed FIX Message
    string error;

    if (MessageValidator::validate(fixMessage, msg, error))
    {
        cout << "\nFIX Message is VALID\n";
    }
    else
    {
        cout << "\nValidation Failed : " << error << endl;
    }
    cout << endl;

    cout
        << "Calculated BodyLength = "
        << BodyLength::calculate(fixMessage)
        << endl;

    return 0;
}