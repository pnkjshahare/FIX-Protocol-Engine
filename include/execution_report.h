#ifndef EXECUTION_REPORT_H
#define EXECUTION_REPORT_H

#include <string>

struct ExecutionReport
{
    std::string orderID;
    std::string execID;
    std::string clOrdID;
    std::string symbol;

    char side;

    int quantity;

    double price;

    char execType; // 0 = New

    char ordStatus; // 0 = New
};

#endif