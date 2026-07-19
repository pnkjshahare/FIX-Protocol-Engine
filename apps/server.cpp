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
#include "tags.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;
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

    //-------------------------------
    // Accept Client
    //-------------------------------
    SOCKET clientSocket =
        accept(serverSocket, nullptr, nullptr);

    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Accept failed\n";

        closesocket(serverSocket);
        WSACleanup();

        return 1;
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

    while (true)
    {
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

        cout << "\n====================================\n";
        cout << " Received FIX Message\n";
        cout << "====================================\n";

        cout << formatForDisplay(networkMessage) << endl;

        //--------------------------------
        // Parse
        //--------------------------------
        FixMessage fixMessage =
            parser.parse(networkMessage);

        cout << "\n====================================\n";
        cout << " Parsed FIX Message\n";
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

        cout << "\nValidation Successful\n";

        //--------------------------------
        // Message Type
        //--------------------------------
        string msgType = fixMessage.getTag(35);

        //--------------------------------
        // Logon
        //--------------------------------
        if (msgType == "A")
        {
            cout << "\nProcessing Logon...\n";

            session.logon();

            string ack =
                encoder.encodeLogon(session);

            cout << "\nSending Logon ACK\n";
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
            cout << "\nProcessing New Order...\n";

            ExecutionReport report;

            report.orderID = "ORDER0001";
            report.execID = "EXEC0001";

            report.clOrdID =
                fixMessage.getTag(FIXTags::ClOrdID);

            report.symbol =
                fixMessage.getTag(FIXTags::Symbol);

            report.side =
                fixMessage.getTag(FIXTags::Side)[0];

            report.quantity =
                stoi(fixMessage.getTag(FIXTags::OrderQty));

            report.price =
                stod(fixMessage.getTag(FIXTags::Price));

            report.execType = '0';

            report.ordStatus = '0';

            string response =
                encoder.encodeExecutionReport(
                    report,
                    session);

            cout << "\nSending Execution Report\n";

            cout << formatForDisplay(response)
                 << endl;

            send(clientSocket,
                 response.c_str(),
                 response.length(),
                 0);
        }

        //--------------------------------
        // Heartbeat
        //--------------------------------
        else if (msgType == "0")
        {
            cout << "\nHeartbeat Received\n";
        }

        //--------------------------------
        // Logout
        //--------------------------------
        else if (msgType == "5")
        {
            cout << "\nLogout Received\n";

            string logoutAck =
                encoder.encodeLogout(session);

            send(clientSocket,
                 logoutAck.c_str(),
                 logoutAck.length(),
                 0);

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
    //-------------------------------
    // Cleanup
    //-------------------------------
    closesocket(clientSocket);
    closesocket(serverSocket);

    WSACleanup();

    return 0;
}