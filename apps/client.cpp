#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <unordered_map>
#include <winsock2.h>

#include "encoder.h"
#include "parser.h"
#include "message_validator.h"
#include "fix_message.h"
#include "session.h"
#include "message.h"
#include "tags.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum class SessionResult
{
    Reconnect,
    Stop
};

enum class ClientAction
{
    NewOrder,
    CancelOrder,
    ModifyOrder,
    ShowOrderBook,
    Logout
};

struct ClientOrderView
{
    string orderID;
    string clOrdID;
    string origClOrdID;
    string symbol;
    char side;
    int quantity;
    double price;
    char ordStatus;
};

string formatForDisplay(const string &message)
{
    string display = message;

    replace(display.begin(),
            display.end(),
            '\x01',
            '|');

    return display;
}

bool sendFix(SOCKET socket,
             const string &message)
{
    int bytesSent =
        send(socket,
             message.c_str(),
             message.length(),
             0);

    return bytesSent != SOCKET_ERROR;
}

SOCKET connectToExchange()
{
    SOCKET clientSocket =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";
        return INVALID_SOCKET;
    }

    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5001);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    cout << "Connecting to Exchange...\n";

    if (connect(clientSocket,
                (sockaddr *)&serverAddr,
                sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "Connection failed\n";
        closesocket(clientSocket);
        return INVALID_SOCKET;
    }

    cout << "Connected Successfully!\n";
    return clientSocket;
}

string nextClientOrderID(const string &prefix,
                         int &counter)
{
    stringstream stream;

    stream << prefix
           << counter++;

    return stream.str();
}

ClientAction promptAction()
{
    int choice;

    cout << "\n========== Trading Menu ==========\n";
    cout << "1. New Order\n";
    cout << "2. Cancel Order\n";
    cout << "3. Modify Order\n";
    cout << "4. Show Order Book\n";
    cout << "5. Logout\n";
    cout << "Enter choice: ";
    cin >> choice;

    if (choice == 2)
    {
        return ClientAction::CancelOrder;
    }

    if (choice == 3)
    {
        return ClientAction::ModifyOrder;
    }

    if (choice == 4)
    {
        return ClientAction::ShowOrderBook;
    }

    if (choice == 5)
    {
        return ClientAction::Logout;
    }

    return ClientAction::NewOrder;
}

char promptSide()
{
    int sideChoice;

    cout << "1. Buy\n";
    cout << "2. Sell\n";
    cout << "Enter side: ";
    cin >> sideChoice;

    return sideChoice == 2 ? '2' : '1';
}

Order promptNewOrder(int &orderCounter)
{
    Order order;

    order.clOrdID =
        nextClientOrderID("ORD", orderCounter);

    order.side = promptSide();

    cout << "Enter Symbol   : ";
    cin >> order.symbol;

    cout << "Enter Quantity : ";
    cin >> order.quantity;

    cout << "Enter Price    : ";
    cin >> order.price;

    return order;
}

CancelOrderRequest promptCancelOrder(int &cancelCounter)
{
    CancelOrderRequest request;

    cout << "Enter ClOrdID to cancel, for example ORD1: ";
    cin >> request.origClOrdID;

    request.clOrdID =
        nextClientOrderID("CANCEL", cancelCounter);

    request.side = promptSide();

    cout << "Enter Symbol   : ";
    cin >> request.symbol;

    cout << "Enter Quantity : ";
    cin >> request.quantity;

    return request;
}

string orderStatusName(char status)
{
    if (status == '0')
    {
        return "New";
    }

    if (status == '4')
    {
        return "Canceled";
    }

    if (status == '8')
    {
        return "Rejected";
    }

    return "Unknown";
}

string msgTypeName(const string &msgType)
{
    if (msgType == "A")
    {
        return "Logon ACK";
    }

    if (msgType == "8")
    {
        return "Execution Report";
    }

    if (msgType == "0")
    {
        return "Heartbeat";
    }

    if (msgType == "1")
    {
        return "Test Request";
    }

    if (msgType == "5")
    {
        return "Logout";
    }

    return "FIX Message";
}

string execTypeName(char execType)
{
    if (execType == '0')
    {
        return "Order Accepted";
    }

    if (execType == '4')
    {
        return "Order Canceled";
    }

    if (execType == '5')
    {
        return "Order Modified";
    }

    if (execType == '8')
    {
        return "Order Rejected";
    }

    return "Execution Report";
}

string inboundMessageTitle(const FixMessage &message)
{
    string msgType =
        message.getTag(FIXTags::MsgType);

    if (msgType == "8" && message.hasTag(FIXTags::ExecType))
    {
        return execTypeName(
            message.getTag(FIXTags::ExecType)[0]) +
            " Execution Report";
    }

    if (msgType == "0" && message.hasTag(FIXTags::TestReqID))
    {
        return "Heartbeat Response for Test Request";
    }

    return msgTypeName(msgType);
}

void updateClientOrderBook(unordered_map<string, ClientOrderView> &orders,
                           const FixMessage &executionReport)
{
    ClientOrderView order;

    order.orderID = executionReport.getTag(FIXTags::OrderID);
    order.clOrdID = executionReport.getTag(FIXTags::ClOrdID);
    order.origClOrdID = executionReport.getTag(FIXTags::OrigClOrdID);
    order.symbol = executionReport.getTag(FIXTags::Symbol);
    order.side = executionReport.getTag(FIXTags::Side)[0];
    order.quantity = stoi(executionReport.getTag(FIXTags::OrderQty));
    order.price = stod(executionReport.getTag(FIXTags::Price));
    order.ordStatus = executionReport.getTag(FIXTags::OrdStatus)[0];

    if (!order.origClOrdID.empty())
    {
        orders.erase(order.origClOrdID);
    }

    orders[order.clOrdID] = order;
}

void printClientOrderBook(
    const unordered_map<string, ClientOrderView> &orders)
{
    cout << "\n========== Local Order Book ==========\n";

    if (orders.empty())
    {
        cout << "No orders available\n";
        return;
    }

    for (const auto &entry : orders)
    {
        const ClientOrderView &order = entry.second;

        cout << "OrderID=" << order.orderID
             << " ClOrdID=" << order.clOrdID;

        if (!order.origClOrdID.empty())
        {
            cout << " OrigClOrdID=" << order.origClOrdID;
        }

        cout << " Symbol=" << order.symbol
             << " Side=" << order.side
             << " Qty=" << order.quantity
             << " Price=" << order.price
             << " Status=" << orderStatusName(order.ordStatus)
             << endl;
    }
}

string resolveClientOrderID(
    const unordered_map<string, ClientOrderView> &orders,
    const string &enteredID)
{
    if (orders.find(enteredID) != orders.end())
    {
        return enteredID;
    }

    for (const auto &entry : orders)
    {
        const ClientOrderView &order = entry.second;

        if (order.orderID == enteredID)
        {
            return order.clOrdID;
        }
    }

    return enteredID;
}

ModifyOrderRequest promptModifyOrder(int &replaceCounter)
{
    ModifyOrderRequest request;

    cout << "Enter ClOrdID to modify, for example ORD1: ";
    cin >> request.origClOrdID;

    request.clOrdID =
        nextClientOrderID("REPLACE", replaceCounter);

    request.side = promptSide();

    cout << "Enter Symbol       : ";
    cin >> request.symbol;

    cout << "Enter New Quantity : ";
    cin >> request.quantity;

    cout << "Enter New Price    : ";
    cin >> request.price;

    return request;
}

bool receiveAndValidate(SOCKET clientSocket,
                        Parser &parser,
                        Session &session,
                        FixMessage &fixMessage,
                        string &rawMessage)
{
    char buffer[2048] = {0};

    int bytesReceived =
        recv(clientSocket,
             buffer,
             sizeof(buffer) - 1,
             0);

    if (bytesReceived <= 0)
    {
        cout << "\nServer Disconnected.\n";
        return false;
    }

    buffer[bytesReceived] = '\0';
    rawMessage = buffer;

    fixMessage =
        parser.parse(rawMessage);

    string title =
        inboundMessageTitle(fixMessage);

    cout << "\n====================================\n";
    cout << " Received " << title << "\n";
    cout << "====================================\n";
    cout << formatForDisplay(rawMessage) << endl;

    cout << "\n====================================\n";
    cout << " Parsed " << title << "\n";
    cout << "====================================\n";
    fixMessage.print();

    string error;

    if (!MessageValidator::validate(
            rawMessage,
            fixMessage,
            error))
    {
        cout << "\nValidation Failed\n";
        cout << error << endl;
        return false;
    }

    if (!session.validateIncomingSeqNum(
            stoi(fixMessage.getTag(FIXTags::MsgSeqNum)),
            error))
    {
        cout << "\nSession Validation Failed\n";
        cout << error << endl;
        return false;
    }

    return true;
}

SessionResult runFixSession(SOCKET clientSocket,
                            bool logoutAfterExecutionReport,
                            bool cancelAfterExecutionReport,
                            bool modifyAfterExecutionReport,
                            bool interactiveMode,
                            unordered_map<string, ClientOrderView> &localOrderBook)
{
    Session session;
    Encoder encoder;
    Parser parser;
    bool logoutSent = false;

    string logon =
        encoder.encodeLogon(session);

    cout << "\n====================================\n";
    cout << " Sending Logon Request (35=A)\n";
    cout << "====================================\n";
    cout << formatForDisplay(logon) << endl;

    if (!sendFix(clientSocket, logon))
    {
        cout << "Logon send failed\n";
        return SessionResult::Reconnect;
    }

    cout << "\nLogon Sent Successfully\n";

    FixMessage ack;
    string ackRaw;

    if (!receiveAndValidate(
            clientSocket,
            parser,
            session,
            ack,
            ackRaw))
    {
        return SessionResult::Reconnect;
    }

    if (ack.getTag(FIXTags::MsgType) != "A")
    {
        cout << "\nExpected Logon ACK\n";
        return SessionResult::Reconnect;
    }

    session.logon();

    cout << "\n====================================\n";
    cout << " Exchange Logon Successful\n";
    cout << "====================================\n";

    int orderCounter = 1;
    int cancelCounter = 1;
    int replaceCounter = 1;

    if (!interactiveMode)
    {
        Order order;

        order.clOrdID = "ORD000001";
        order.symbol = "RELIANCE";
        order.quantity = 100;
        order.price = 2500.50;
        order.side = '1';

        string orderMessage =
            encoder.encode(order, session);

        cout << "\n====================================\n";
        cout << " Sending New Order Request"
             << " ClOrdID=" << order.clOrdID
             << " Symbol=" << order.symbol
             << " Qty=" << order.quantity
             << " Price=" << order.price
             << "\n";
        cout << "====================================\n";
        cout << formatForDisplay(orderMessage) << endl;

        if (!sendFix(clientSocket, orderMessage))
        {
            cout << "New Order send failed\n";
            return SessionResult::Reconnect;
        }

        cout << "\nNew Order Sent Successfully\n";
    }

    bool pendingTestRequest = false;
    string pendingTestReqID;
    int testRequestCounter = 1;
    bool orderActionSent = false;
    bool interactiveAwaitingResponse = false;

    cout << "\n====================================\n";
    cout << " FIX Session Active\n";
    cout << " Waiting for execution reports and admin messages\n";
    cout << "====================================\n";

    while (true)
    {
        if (interactiveMode && !interactiveAwaitingResponse)
        {
            ClientAction action =
                promptAction();

            if (action == ClientAction::NewOrder)
            {
                Order order =
                    promptNewOrder(orderCounter);

                string orderMessage =
                    encoder.encode(order, session);

                cout << "\n====================================\n";
                cout << " Sending New Order Request"
                     << " ClOrdID=" << order.clOrdID
                     << " Symbol=" << order.symbol
                     << " Qty=" << order.quantity
                     << " Price=" << order.price
                     << "\n";
                cout << "====================================\n";
                cout << formatForDisplay(orderMessage)
                     << endl;

                if (!sendFix(clientSocket, orderMessage))
                {
                    cout << "New Order send failed\n";
                    return SessionResult::Reconnect;
                }

                interactiveAwaitingResponse = true;
            }
            else if (action == ClientAction::CancelOrder)
            {
                if (localOrderBook.empty())
                {
                    cout << "\nCannot cancel because order book is empty.\n";
                    printClientOrderBook(localOrderBook);
                    continue;
                }

                CancelOrderRequest request =
                    promptCancelOrder(cancelCounter);

                request.origClOrdID =
                    resolveClientOrderID(
                        localOrderBook,
                        request.origClOrdID);

                string cancelMessage =
                    encoder.encodeCancelOrder(
                        request,
                        session);

                cout << "\n====================================\n";
                cout << " Sending Cancel Order Request"
                     << " OrigClOrdID=" << request.origClOrdID
                     << " CancelClOrdID=" << request.clOrdID
                     << "\n";
                cout << "====================================\n";
                cout << formatForDisplay(cancelMessage)
                     << endl;

                if (!sendFix(clientSocket, cancelMessage))
                {
                    cout << "Cancel Order send failed\n";
                    return SessionResult::Reconnect;
                }

                interactiveAwaitingResponse = true;
            }
            else if (action == ClientAction::ModifyOrder)
            {
                if (localOrderBook.empty())
                {
                    cout << "\nCannot modify because order book is empty.\n";
                    printClientOrderBook(localOrderBook);
                    continue;
                }

                ModifyOrderRequest request =
                    promptModifyOrder(replaceCounter);

                request.origClOrdID =
                    resolveClientOrderID(
                        localOrderBook,
                        request.origClOrdID);

                string modifyMessage =
                    encoder.encodeModifyOrder(
                        request,
                        session);

                cout << "\n====================================\n";
                cout << " Sending Modify Order Request"
                     << " OrigClOrdID=" << request.origClOrdID
                     << " NewClOrdID=" << request.clOrdID
                     << " NewQty=" << request.quantity
                     << " NewPrice=" << request.price
                     << "\n";
                cout << "====================================\n";
                cout << formatForDisplay(modifyMessage)
                     << endl;

                if (!sendFix(clientSocket, modifyMessage))
                {
                    cout << "Modify Order send failed\n";
                    return SessionResult::Reconnect;
                }

                interactiveAwaitingResponse = true;
            }
            else if (action == ClientAction::ShowOrderBook)
            {
                printClientOrderBook(localOrderBook);
                continue;
            }
            else
            {
                string logout =
                    encoder.encodeLogout(session);

                cout << "\n====================================\n";
                cout << " Sending Logout Request\n";
                cout << "====================================\n";
                cout << formatForDisplay(logout)
                     << endl;

                if (!sendFix(clientSocket, logout))
                {
                    cout << "Logout send failed\n";
                    return SessionResult::Reconnect;
                }

                logoutSent = true;
                session.logout();
                interactiveAwaitingResponse = true;
            }
        }

        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);

        timeval timeout;
        timeout.tv_sec = session.getHeartBtInt();
        timeout.tv_usec = 0;

        int ready =
            select(0,
                   &readSet,
                   nullptr,
                   nullptr,
                   &timeout);

        if (ready == SOCKET_ERROR)
        {
            cout << "\nSocket select failed\n";
            return SessionResult::Reconnect;
        }

        if (ready == 0)
        {
            if (pendingTestRequest)
            {
                cout << "\nNo heartbeat received for TestReqID "
                     << pendingTestReqID
                     << ". Reconnecting.\n";
                return SessionResult::Reconnect;
            }

            pendingTestReqID =
                "CLIENT_TEST_" +
                to_string(testRequestCounter++);

            string testRequest =
                encoder.encodeTestRequest(
                    session,
                    "CLIENT1",
                    "EXCHANGE",
                    pendingTestReqID);

            pendingTestRequest = true;

            cout << "\nNo inbound message for "
                 << session.getHeartBtInt()
                 << " seconds\n";
            cout << "Sending Test Request "
                 << pendingTestReqID
                 << "\n";
            cout << formatForDisplay(testRequest)
                 << endl;

            if (!sendFix(clientSocket, testRequest))
            {
                cout << "Test Request send failed\n";
                return SessionResult::Reconnect;
            }

            continue;
        }

        FixMessage fixMessage;
        string rawMessage;

        if (!receiveAndValidate(
                clientSocket,
                parser,
                session,
                fixMessage,
                rawMessage))
        {
            return SessionResult::Reconnect;
        }

        string msgType =
            fixMessage.getTag(FIXTags::MsgType);

        if (msgType == "8")
        {
            updateClientOrderBook(localOrderBook, fixMessage);

            cout << "\n====================================\n";
            cout << " Execution Report Received\n";
            cout << "====================================\n";
            cout << "ClOrdID="
                 << fixMessage.getTag(FIXTags::ClOrdID)
                 << " ExecType="
                 << fixMessage.getTag(FIXTags::ExecType)
                 << " OrdStatus="
                 << orderStatusName(
                        fixMessage.getTag(FIXTags::OrdStatus)[0])
                 << endl;

            interactiveAwaitingResponse = false;

            if (!orderActionSent && cancelAfterExecutionReport)
            {
                CancelOrderRequest cancelRequest;

                cancelRequest.origClOrdID = "ORD000001";
                cancelRequest.clOrdID = "CANCEL000001";
                cancelRequest.symbol = "RELIANCE";
                cancelRequest.quantity = 100;
                cancelRequest.side = '1';

                string cancelMessage =
                    encoder.encodeCancelOrder(
                        cancelRequest,
                        session);

                cout << "\n====================================\n";
                cout << " Sending Cancel Order Request"
                     << " OrigClOrdID=" << cancelRequest.origClOrdID
                     << " CancelClOrdID=" << cancelRequest.clOrdID
                     << "\n";
                cout << "====================================\n";
                cout << formatForDisplay(cancelMessage)
                     << endl;

                if (!sendFix(clientSocket, cancelMessage))
                {
                    cout << "Cancel Order send failed\n";
                    return SessionResult::Reconnect;
                }

                orderActionSent = true;
                continue;
            }

            if (!orderActionSent && modifyAfterExecutionReport)
            {
                ModifyOrderRequest modifyRequest;

                modifyRequest.origClOrdID = "ORD000001";
                modifyRequest.clOrdID = "REPLACE000001";
                modifyRequest.symbol = "RELIANCE";
                modifyRequest.quantity = 150;
                modifyRequest.price = 2499.75;
                modifyRequest.side = '1';

                string modifyMessage =
                    encoder.encodeModifyOrder(
                        modifyRequest,
                        session);

                cout << "\n====================================\n";
                cout << " Sending Modify Order Request"
                     << " OrigClOrdID=" << modifyRequest.origClOrdID
                     << " NewClOrdID=" << modifyRequest.clOrdID
                     << "\n";
                cout << "====================================\n";
                cout << formatForDisplay(modifyMessage)
                     << endl;

                if (!sendFix(clientSocket, modifyMessage))
                {
                    cout << "Modify Order send failed\n";
                    return SessionResult::Reconnect;
                }

                orderActionSent = true;
                continue;
            }

            if (logoutAfterExecutionReport && !logoutSent)
            {
                string logout =
                    encoder.encodeLogout(session);

                cout << "\n====================================\n";
                cout << " Sending Logout Request\n";
                cout << "====================================\n";
                cout << formatForDisplay(logout)
                     << endl;

                if (!sendFix(clientSocket, logout))
                {
                    cout << "Logout send failed\n";
                    return SessionResult::Reconnect;
                }

                logoutSent = true;
                session.logout();
            }
        }
        else if (msgType == "0")
        {
            cout << "\nHeartbeat Received\n";

            if (pendingTestRequest &&
                fixMessage.getTag(FIXTags::TestReqID) ==
                    pendingTestReqID)
            {
                pendingTestRequest = false;
                pendingTestReqID.clear();
                cout << "Test Request satisfied\n";
            }
        }
        else if (msgType == "1")
        {
            cout << "\nTest Request Received\n";

            string heartbeat =
                encoder.encodeHeartbeat(
                    session,
                    "CLIENT1",
                    "EXCHANGE",
                    fixMessage.getTag(FIXTags::TestReqID));

            cout << "Sending Heartbeat Response for TestReqID="
                 << fixMessage.getTag(FIXTags::TestReqID)
                 << "\n";
            cout << formatForDisplay(heartbeat)
                 << endl;

            if (!sendFix(clientSocket, heartbeat))
            {
                cout << "Heartbeat send failed\n";
                return SessionResult::Reconnect;
            }
        }
        else if (msgType == "5")
        {
            cout << "\nLogout Received\n";

            if (!logoutSent)
            {
                string logoutAck =
                    encoder.encodeLogout(session);

                cout << "Sending Logout ACK (35=5)\n";
                cout << formatForDisplay(logoutAck)
                     << endl;

                sendFix(clientSocket, logoutAck);
            }

            session.logout();
            return SessionResult::Stop;
        }
        else
        {
            cout << "\nReceived MsgType : "
                 << msgType
                 << endl;
        }
    }
}

int main(int argc, char *argv[])
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";
        return 1;
    }

    const int RECONNECT_DELAY_SECONDS = 3;
    bool logoutAfterExecutionReport = false;
    bool cancelAfterExecutionReport = false;
    bool modifyAfterExecutionReport = false;
    bool interactiveMode = false;
    unordered_map<string, ClientOrderView> localOrderBook;

    for (int i = 1; i < argc; ++i)
    {
        string argument = argv[i];

        if (argument == "--logout-after-report")
        {
            logoutAfterExecutionReport = true;
        }
        else if (argument == "--cancel-after-report")
        {
            cancelAfterExecutionReport = true;
        }
        else if (argument == "--modify-after-report")
        {
            modifyAfterExecutionReport = true;
        }
        else if (argument == "--interactive")
        {
            interactiveMode = true;
        }
    }

    while (true)
    {
        SOCKET clientSocket =
            connectToExchange();

        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Retrying in "
                 << RECONNECT_DELAY_SECONDS
                 << " seconds...\n";
            Sleep(RECONNECT_DELAY_SECONDS * 1000);
            continue;
        }

        SessionResult result =
            runFixSession(
                clientSocket,
                logoutAfterExecutionReport,
                cancelAfterExecutionReport,
                modifyAfterExecutionReport,
                interactiveMode,
                localOrderBook);

        closesocket(clientSocket);

        if (result == SessionResult::Stop)
        {
            cout << "\nFIX session closed cleanly.\n";
            break;
        }

        cout << "\nReconnecting in "
             << RECONNECT_DELAY_SECONDS
             << " seconds...\n";

        Sleep(RECONNECT_DELAY_SECONDS * 1000);
    }

    WSACleanup();

    return 0;
}
