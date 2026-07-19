#include <iostream>
#include <algorithm>
#include <winsock2.h>

#include "encoder.h"
#include "parser.h"
#include "message_validator.h"
#include "fix_message.h"
#include "session.h"
#include "message.h"

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
    //----------------------------------
    // Initialize Winsock
    //----------------------------------
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "WSAStartup failed\n";
        return 1;
    }

    //----------------------------------
    // Create Socket
    //----------------------------------
    SOCKET clientSocket =
        socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    //----------------------------------
    // Server Address
    //----------------------------------
    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5001);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    cout << "Connecting to Exchange...\n";

    //----------------------------------
    // Connect
    //----------------------------------
    if (connect(clientSocket,
                (sockaddr *)&serverAddr,
                sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cout << "Connection failed\n";

        closesocket(clientSocket);
        WSACleanup();

        return 1;
    }

    cout << "Connected Successfully!\n";

    //----------------------------------
    // FIX Components
    //----------------------------------
    Session session;
    Encoder encoder;
    Parser parser;

    //----------------------------------
    // Encode Logon
    //----------------------------------
    string message =
        encoder.encodeLogon(session);

    cout << "\n====================================\n";
    cout << " Sending Logon\n";
    cout << "====================================\n";
    cout << formatForDisplay(message) << endl;
    //----------------------------------
    // Send
    //----------------------------------
    send(clientSocket,
         message.c_str(),
         message.length(),
         0);

    cout << "\nLogon Sent Successfully\n";

    //----------------------------------
    // Receive ACK
    //----------------------------------
    char buffer[2048] = {0};

    int bytesReceived =
        recv(clientSocket,
             buffer,
             sizeof(buffer) - 1,
             0);

    if (bytesReceived > 0)

    {
        buffer[bytesReceived] = '\0';

        string ack(buffer);

        cout << "\n====================================\n";
        cout << " Received ACK\n";
        cout << "====================================\n";

        cout << formatForDisplay(ack) << endl;

        //----------------------------------
        // Convert | to SOH
        //----------------------------------
        string parserMessage = ack;

        //----------------------------------
        // Parse ACK
        //----------------------------------
        FixMessage fixMessage =
            parser.parse(parserMessage);

        cout << "\n====================================\n";
        cout << " Parsed ACK\n";
        cout << "====================================\n";

        fixMessage.print();

        //----------------------------------
        // Validate ACK
        //----------------------------------
        string error;

        if (MessageValidator::validate(
                parserMessage,
                fixMessage,
                error))
        {
            cout << "\n====================================\n";
            cout << " Exchange Logon Successful\n";
            cout << "====================================\n";

            //----------------------------------
            // Create New Order
            //----------------------------------
            Order order;

            order.clOrdID = "ORD000001";
            order.symbol = "RELIANCE";
            order.quantity = 100;
            order.price = 2500.50;
            order.side = '1'; // Buy

            //----------------------------------
            // Encode Order
            //----------------------------------
            string orderMessage =
                encoder.encode(order, session);

            cout << "\n====================================\n";
            cout << " Sending New Order\n";
            cout << "====================================\n";

            cout << formatForDisplay(orderMessage) << endl;

            //----------------------------------
            // Send Order
            //----------------------------------
            send(clientSocket,
                 orderMessage.c_str(),
                 orderMessage.length(),
                 0);

            cout << "\nNew Order Sent Successfully\n";
            //----------------------------------
            // Receive Execution Report
            //----------------------------------
            char reportBuffer[2048] = {0};

            int reportBytes =
                recv(clientSocket,
                     reportBuffer,
                     sizeof(reportBuffer) - 1,
                     0);

            if (reportBytes > 0)
            {
                reportBuffer[reportBytes] = '\0';

                string report(reportBuffer);

                cout << "\n====================================\n";
                cout << " Received Execution Report\n";
                cout << "====================================\n";

                cout << formatForDisplay(report) << endl;

                //----------------------------------
                // Parse
                //----------------------------------
                FixMessage execReport =
                    parser.parse(report);

                cout << "\n====================================\n";
                cout << " Parsed Execution Report\n";
                cout << "====================================\n";

                execReport.print();

                //----------------------------------
                // Validate
                //----------------------------------
                string execError;

                if (MessageValidator::validate(
                        report,
                        execReport,
                        execError))
                {
                    cout << "\n====================================\n";
                    cout << " Order Accepted By Exchange\n";
                    cout << "====================================\n";
                }
                else
                {
                    cout << "\nExecution Report Invalid\n";
                    cout << execError << endl;
                }
            }
        }
        else
        {
            cout << "\n====================================\n";
            cout << " Invalid ACK\n";
            cout << "====================================\n";

            cout << error << endl;
        }
        //----------------------------------
        // Cleanup
        //----------------------------------
        closesocket(clientSocket);

        WSACleanup();

        return 0;
    }
}