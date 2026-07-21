#include <iostream>
#include <algorithm>
#include <string>
#include <winsock2.h>

#include "parser.h"
#include "fix_message.h"
#include "message_validator.h"
#include "encoder.h"
#include "session.h"
#include "execution_report.h"
#include "order_book.h"
#include "tags.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;

string msgTypeName(const string &msgType)
{
    if (msgType == "A")
    {
        return "Logon";
    }

    if (msgType == "D")
    {
        return "New Order";
    }

    if (msgType == "F")
    {
        return "Cancel Order";
    }

    if (msgType == "G")
    {
        return "Modify Order";
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

    return "Unknown";
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

void printInboundSummary(const FixMessage &message)
{
    string msgType =
        message.getTag(FIXTags::MsgType);

    cout << "\nRequest: "
         << msgTypeName(msgType)
         << " (35=" << msgType << ")\n";

    cout << "SeqNum=" << message.getTag(FIXTags::MsgSeqNum)
         << " Sender=" << message.getTag(FIXTags::SenderCompID)
         << " Target=" << message.getTag(FIXTags::TargetCompID)
         << endl;

    if (msgType == "D")
    {
        cout << "New Order Details: "
             << "ClOrdID=" << message.getTag(FIXTags::ClOrdID)
             << " Symbol=" << message.getTag(FIXTags::Symbol)
             << " Side=" << message.getTag(FIXTags::Side)
             << " Qty=" << message.getTag(FIXTags::OrderQty)
             << " Price=" << message.getTag(FIXTags::Price)
             << endl;
    }
    else if (msgType == "F")
    {
        cout << "Cancel Details: "
             << "OrigClOrdID=" << message.getTag(FIXTags::OrigClOrdID)
             << " CancelClOrdID=" << message.getTag(FIXTags::ClOrdID)
             << " Symbol=" << message.getTag(FIXTags::Symbol)
             << " Qty=" << message.getTag(FIXTags::OrderQty)
             << endl;
    }
    else if (msgType == "G")
    {
        cout << "Modify Details: "
             << "OrigClOrdID=" << message.getTag(FIXTags::OrigClOrdID)
             << " NewClOrdID=" << message.getTag(FIXTags::ClOrdID)
             << " Symbol=" << message.getTag(FIXTags::Symbol)
             << " NewQty=" << message.getTag(FIXTags::OrderQty)
             << " NewPrice=" << message.getTag(FIXTags::Price)
             << endl;
    }
    else if (msgType == "1")
    {
        cout << "TestReqID="
             << message.getTag(FIXTags::TestReqID)
             << endl;
    }
    else if (msgType == "0" && message.hasTag(FIXTags::TestReqID))
    {
        cout << "Heartbeat TestReqID="
             << message.getTag(FIXTags::TestReqID)
             << endl;
    }
}

string formatForDisplay(const string &message)
{
    string display = message;

    replace(display.begin(),
            display.end(),
            '\x01',
            '|');

    return display;
}
int main()
{
    //-------------------------------
    // Initialize Winsock
    //-------------------------------
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";
        return 1;
    }

    //-------------------------------
    // Create Server Socket
    //-------------------------------
    SOCKET serverSocket =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    //-------------------------------
    // Configure Address
    //-------------------------------
    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5001);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //-------------------------------
    // Bind
    //-------------------------------
    if (bind(serverSocket,
             (sockaddr *)&serverAddr,
             sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "Bind failed\n";

        closesocket(serverSocket);
        WSACleanup();

        return 1;
    }

    //-------------------------------
    // Listen
    //-------------------------------
    listen(serverSocket, 1);

    cout << "====================================\n";
    cout << " FIX Exchange Server Started\n";
    cout << " Waiting for client...\n";
    cout << "====================================\n";

    OrderBook orderBook;
    int execCounter = 1;

    while (true)
    {
        //-------------------------------
        // Accept Client
        //-------------------------------
        SOCKET clientSocket =
            accept(serverSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Accept failed\n";
            break;
        }

        cout << "\nClient Connected!\n";

        //-------------------------------
        // FIX Components
        //-------------------------------
        Session session;
        Parser parser;
        Encoder encoder;

        //--------------------------------
        // Receive Messages
        //--------------------------------
        char buffer[2048];
        bool pendingTestRequest = false;
        string pendingTestReqID;
        int testRequestCounter = 1;

        while (true)
        {
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
                break;
            }

            if (ready == 0)
            {
                if (!session.isLoggedOn())
                {
                    cout << "\nLogon timeout\n";
                    break;
                }

                if (pendingTestRequest)
                {
                    cout << "\nNo heartbeat received for TestReqID "
                         << pendingTestReqID
                         << ". Closing session.\n";
                    break;
                }

                pendingTestReqID =
                    "SERVER_TEST_" +
                    to_string(testRequestCounter++);

                string testRequest =
                    encoder.encodeTestRequest(
                        session,
                        "EXCHANGE",
                        "CLIENT1",
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

                send(clientSocket,
                     testRequest.c_str(),
                     testRequest.length(),
                     0);

                continue;
            }

            memset(buffer, 0, sizeof(buffer));

            int bytesReceived =
                recv(clientSocket,
                     buffer,
                     sizeof(buffer) - 1,
                     0);

            if (bytesReceived <= 0)
            {
                cout << "\nClient Disconnected.\n";
                break;
            }

            buffer[bytesReceived] = '\0';

            string networkMessage(buffer);

            //--------------------------------
            // Parse
            //--------------------------------
            FixMessage fixMessage =
                parser.parse(networkMessage);

            string receivedMsgType =
                fixMessage.getTag(FIXTags::MsgType);

            cout << "\n====================================\n";
            cout << " Received "
                 << msgTypeName(receivedMsgType)
                 << " Request"
                 << " (35=" << receivedMsgType << ")\n";
            cout << "====================================\n";

            cout << formatForDisplay(networkMessage) << endl;

            cout << "\n====================================\n";
            cout << " Parsed "
                 << msgTypeName(receivedMsgType)
                 << " Request\n";
            cout << "====================================\n";

            fixMessage.print();

            //--------------------------------
            // Validate
            //--------------------------------
            string error;

            if (!MessageValidator::validate(
                    networkMessage,
                    fixMessage,
                    error))
            {
                cout << "\nValidation Failed\n";
                cout << error << endl;
                continue;
            }

            if (!session.validateIncomingSeqNum(
                    stoi(fixMessage.getTag(FIXTags::MsgSeqNum)),
                    error))
            {
                cout << "\nSession Validation Failed\n";
                cout << error << endl;
                break;
            }

            cout << "\nValidation Successful\n";

            //--------------------------------
            // Message Type
            //--------------------------------
            string msgType = fixMessage.getTag(35);
            printInboundSummary(fixMessage);

            //--------------------------------
            // Logon
            //--------------------------------
            if (msgType == "A")
            {
                cout << "\nAction: Accepting logon request\n";

                session.setHeartBtInt(
                    stoi(fixMessage.getTag(FIXTags::HeartBtInt)));

                session.logon();

                string ack =
                    encoder.encodeLogon(
                        session,
                        "EXCHANGE",
                        "CLIENT1");

                cout << "Response: Logon ACK (35=A)\n";
                cout << formatForDisplay(ack) << endl;

                send(clientSocket,
                     ack.c_str(),
                     ack.length(),
                     0);
            }

            //--------------------------------
            // New Order
            //--------------------------------
            else if (msgType == "D")
            {
                cout << "\nAction: Storing new order in order book\n";

                StoredOrder storedOrder =
                    orderBook.addOrder(fixMessage);

                ExecutionReport report;

                report.orderID = storedOrder.orderID;
                report.execID =
                    "EXEC" +
                    to_string(execCounter++);

                report.clOrdID = storedOrder.clOrdID;

                report.symbol = storedOrder.symbol;

                report.side = storedOrder.side;

                report.quantity = storedOrder.quantity;

                report.price = storedOrder.price;

                report.execType = '0';

                report.ordStatus = storedOrder.ordStatus;

                string response =
                    encoder.encodeExecutionReport(
                        report,
                        session);

                cout << "Response: Execution Report Accepted "
                     << "OrderID=" << report.orderID
                     << " ClOrdID=" << report.clOrdID
                     << " Status=" << orderStatusName(report.ordStatus)
                     << endl;

                cout << formatForDisplay(response)
                     << endl;

                send(clientSocket,
                     response.c_str(),
                     response.length(),
                     0);

                orderBook.print();
            }

            //--------------------------------
            // Cancel Order
            //--------------------------------
            else if (msgType == "F")
            {
                cout << "\nAction: Searching order book for cancel request\n";

                StoredOrder storedOrder;
                bool cancelled =
                    orderBook.cancelOrder(
                        fixMessage.getTag(FIXTags::OrigClOrdID),
                        fixMessage.getTag(FIXTags::ClOrdID),
                        storedOrder);

                ExecutionReport report;

                report.orderID =
                    cancelled ? storedOrder.orderID : "UNKNOWN";
                report.execID =
                    "EXEC" +
                    to_string(execCounter++);
                report.clOrdID =
                    fixMessage.getTag(FIXTags::ClOrdID);
                report.origClOrdID =
                    fixMessage.getTag(FIXTags::OrigClOrdID);
                report.symbol =
                    fixMessage.getTag(FIXTags::Symbol);
                report.side =
                    fixMessage.getTag(FIXTags::Side)[0];
                report.quantity =
                    stoi(fixMessage.getTag(FIXTags::OrderQty));
                report.price =
                    cancelled ? storedOrder.price : 0.01;
                report.execType =
                    cancelled ? '4' : '8';
                report.ordStatus =
                    cancelled ? '4' : '8';

                string response =
                    encoder.encodeExecutionReport(
                        report,
                        session);

                cout << "Response: Cancel Execution Report "
                     << "ClOrdID=" << report.clOrdID
                     << " OrigClOrdID=" << report.origClOrdID
                     << " Status=" << orderStatusName(report.ordStatus)
                     << endl;
                cout << formatForDisplay(response)
                     << endl;

                send(clientSocket,
                     response.c_str(),
                     response.length(),
                     0);

                orderBook.print();
            }

            //--------------------------------
            // Modify Order
            //--------------------------------
            else if (msgType == "G")
            {
                cout << "\nAction: Searching order book for modify request\n";

                StoredOrder storedOrder;
                bool modified =
                    orderBook.modifyOrder(
                        fixMessage,
                        storedOrder);

                ExecutionReport report;

                report.orderID =
                    modified ? storedOrder.orderID : "UNKNOWN";
                report.execID =
                    "EXEC" +
                    to_string(execCounter++);
                report.clOrdID =
                    fixMessage.getTag(FIXTags::ClOrdID);
                report.origClOrdID =
                    fixMessage.getTag(FIXTags::OrigClOrdID);
                report.symbol =
                    fixMessage.getTag(FIXTags::Symbol);
                report.side =
                    fixMessage.getTag(FIXTags::Side)[0];
                report.quantity =
                    stoi(fixMessage.getTag(FIXTags::OrderQty));
                report.price =
                    stod(fixMessage.getTag(FIXTags::Price));
                report.execType =
                    modified ? '5' : '8';
                report.ordStatus =
                    modified ? '0' : '8';

                string response =
                    encoder.encodeExecutionReport(
                        report,
                        session);

                cout << "Response: Modify Execution Report "
                     << "ClOrdID=" << report.clOrdID
                     << " OrigClOrdID=" << report.origClOrdID
                     << " Status=" << orderStatusName(report.ordStatus)
                     << endl;
                cout << formatForDisplay(response)
                     << endl;

                send(clientSocket,
                     response.c_str(),
                     response.length(),
                     0);

                orderBook.print();
            }

            //--------------------------------
            // Heartbeat
            //--------------------------------
            else if (msgType == "0")
            {
                cout << "\nAction: Heartbeat received\n";

                if (pendingTestRequest &&
                    fixMessage.getTag(FIXTags::TestReqID) ==
                        pendingTestReqID)
                {
                    pendingTestRequest = false;
                    pendingTestReqID.clear();
                    cout << "Test Request satisfied\n";
                }
            }

            //--------------------------------
            // Test Request
            //--------------------------------
            else if (msgType == "1")
            {
                cout << "\nAction: Test Request received\n";

                string heartbeat =
                    encoder.encodeHeartbeat(
                        session,
                        "EXCHANGE",
                        "CLIENT1",
                        fixMessage.getTag(FIXTags::TestReqID));

                cout << "Response: Heartbeat for TestReqID="
                     << fixMessage.getTag(FIXTags::TestReqID)
                     << "\n";
                cout << formatForDisplay(heartbeat)
                     << endl;

                send(clientSocket,
                     heartbeat.c_str(),
                     heartbeat.length(),
                     0);
            }

            //--------------------------------
            // Logout
            //--------------------------------
            else if (msgType == "5")
            {
                cout << "\nAction: Logout request received\n";

                string logoutAck =
                    encoder.encodeLogout(
                        session,
                        "EXCHANGE",
                        "CLIENT1");

                cout << "Response: Logout ACK (35=5)\n";
                cout << formatForDisplay(logoutAck)
                     << endl;

                send(clientSocket,
                     logoutAck.c_str(),
                     logoutAck.length(),
                     0);

                session.logout();
                cout << "FIX session closed cleanly\n";
                break;
            }

            //--------------------------------
            // Unknown Message
            //--------------------------------
            else
            {
                cout << "\nUnsupported MsgType : "
                     << msgType
                     << endl;
            }
        }

        closesocket(clientSocket);
        cout << "\nWaiting for client...\n";
    }
    //-------------------------------
    // Cleanup
    //-------------------------------
    closesocket(serverSocket);

    WSACleanup();

    return 0;
}
