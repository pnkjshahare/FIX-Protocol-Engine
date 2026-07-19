#include <iostream>

#include "body_length.h"
#include "encoder.h"
#include "message.h"
#include "message_validator.h"
#include "parser.h"
#include "session.h"
#include "validator.h"

using namespace std;

int main()
{
    Order order;
    int choice;

    // =========================
    // User Menu
    // =========================
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

    // =========================
    // Take Order Details
    // =========================
    cout << "\nEnter Stock Name : ";
    cin >> order.symbol;

    cout << "Enter Quantity   : ";
    cin >> order.quantity;

    cout << "Enter Price      : ";
    cin >> order.price;

    order.clOrdID = "ORD000001";

    // =========================
    // Validate Order
    // =========================
    string errorMessage;

    if (!Validator::validate(order, errorMessage))
    {
        cout << "\nValidation Error: "
             << errorMessage << endl;
        return 0;
    }

    // =========================
    // Create Session
    // =========================
    Session session;
    Encoder encoder;
    Parser parser;

    string error;

    // =====================================================
    // LOGON MESSAGE
    // =====================================================
    string logonMessage = encoder.encodeLogon(session);

    cout << "\n========== LOGON ==========\n";
    cout << logonMessage << endl;

    FixMessage logonFix = parser.parse(logonMessage);

    cout << "\nParsed Logon Fields\n";
    logonFix.print();

    error.clear();

    if (MessageValidator::validate(logonMessage, logonFix, error))
    {
        cout << "\nLogon Message is VALID\n";
    }
    else
    {
        cout << "\nValidation Failed: "
             << error << endl;
    }

    cout << "\nCalculated BodyLength = "
         << BodyLength::calculate(logonMessage)
         << endl;

    // =====================================================
    // HEARTBEAT MESSAGE
    // =====================================================
    string heartbeatMessage =
        encoder.encodeHeartbeat(session);

    cout << "\n========== HEARTBEAT ==========\n";
    cout << heartbeatMessage << endl;

    FixMessage heartbeatFix =
        parser.parse(heartbeatMessage);

    cout << "\nParsed Heartbeat Fields\n";
    heartbeatFix.print();

    error.clear();

    if (MessageValidator::validate(
            heartbeatMessage,
            heartbeatFix,
            error))
    {
        cout << "\nHeartbeat Message is VALID\n";
    }
    else
    {
        cout << "\nValidation Failed: "
             << error << endl;
    }

    cout << "\nCalculated BodyLength = "
         << BodyLength::calculate(heartbeatMessage)
         << endl;

    // =====================================================
    // NEW ORDER MESSAGE
    // =====================================================
    string orderMessage =
        encoder.encode(order, session);

    cout << "\n========== NEW ORDER ==========\n";
    cout << orderMessage << endl;

    FixMessage orderFix =
        parser.parse(orderMessage);

    cout << "\nParsed Order Fields\n";
    orderFix.print();

    error.clear();

    if (MessageValidator::validate(
            orderMessage,
            orderFix,
            error))
    {
        cout << "\nOrder Message is VALID\n";
    }
    else
    {
        cout << "\nValidation Failed: "
             << error << endl;
    }

    cout << "\nCalculated BodyLength = "
         << BodyLength::calculate(orderMessage)
         << endl;

    // =====================================================
    // LOGOUT MESSAGE
    // =====================================================
    string logoutMessage =
        encoder.encodeLogout(session);

    cout << "\n========== LOGOUT ==========\n";
    cout << logoutMessage << endl;

    FixMessage logoutFix =
        parser.parse(logoutMessage);

    cout << "\nParsed Logout Fields\n";
    logoutFix.print();

    error.clear();

    if (MessageValidator::validate(
            logoutMessage,
            logoutFix,
            error))
    {
        cout << "\nLogout Message is VALID\n";
    }
    else
    {
        cout << "\nValidation Failed: "
             << error << endl;
    }

    cout << "\nCalculated BodyLength = "
         << BodyLength::calculate(logoutMessage)
         << endl;

    return 0;
}