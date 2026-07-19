#include "utils.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string getCurrentUTCTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm *utc = std::gmtime(&t);

    std::stringstream ss;

    ss << std::put_time(utc, "%Y%m%d-%H:%M:%S");

    return ss.str();
}