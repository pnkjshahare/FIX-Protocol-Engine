#include <iostream>
#include <algorithm>
#include <string>
#include <winsock2.h>

#include "parser.h"
#include "fix_message.h"
#include "message_validator.h"
#include "encoder.h"
#include "session.h"

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

    //-------------------------------
    // Receive Message
    //-------------------------------
    char buffer[2048] = {0};

    int bytesReceived =
        recv(clientSocket,
             buffer,
             sizeof(buffer) - 1,
             0);

    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';

        string networkMessage(buffer);

        cout << "\n====================================\n";
        cout << " Received FIX Message\n";
        cout << "====================================\n";

        // cout << networkMessage << endl;
        cout << formatForDisplay(networkMessage) << endl;

        //---------------------------------------------------
        // Convert | to SOH for parser
        //---------------------------------------------------
        string parserMessage = networkMessage;

        //---------------------------------------------------
        // Parse
        //---------------------------------------------------
        FixMessage fixMessage =
            parser.parse(parserMessage);

        cout << "\n====================================\n";
        cout << " Parsed FIX Message\n";
        cout << "====================================\n";

        fixMessage.print();

        //---------------------------------------------------
        // Validate
        //---------------------------------------------------
        string error;

        if (MessageValidator::validate(
                parserMessage,
                fixMessage,
                error))
        {
            cout << "\n====================================\n";
            cout << " Validation Successful\n";
            cout << "====================================\n";

            //---------------------------------------------------
            // Update Session
            //---------------------------------------------------
            session.logon();

            cout << "Client Logged In\n";

            //---------------------------------------------------
            // Create Logon ACK
            //---------------------------------------------------
            string ack =
                encoder.encodeLogon(session);

            cout << "\n====================================\n";
            cout << " Sending Logon ACK\n";
            cout << "====================================\n";

            // cout << ack << endl;
            cout << formatForDisplay(ack) << endl;
            //---------------------------------------------------
            // Send ACK
            //---------------------------------------------------
            int bytesSent =
                send(clientSocket,
                     ack.c_str(),
                     ack.length(),
                     0);

            if (bytesSent == SOCKET_ERROR)
            {
                cout << "Failed to send ACK\n";
            }
            else
            {
                cout << "ACK Sent Successfully\n";
            }
        }
        else
        {
            cout << "\n====================================\n";
            cout << " Validation Failed\n";
            cout << "====================================\n";

            cout << error << endl;
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