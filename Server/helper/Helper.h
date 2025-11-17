#ifndef SLEEP_HPP_
#define SLEEP_HPP_

#include <thread>
#include <chrono>

void sleep_microseconds(unsigned int us);
void sleep_milliseconds(unsigned int ms);
inline bool FromString(const std::string& value, bool& out)
{
    std::string v = value;
    std::transform(v.begin(), v.end(), v.begin(),
        [](unsigned char c) { return std::tolower(c); });
    if (v == "true" || v == "1") { out = true; return true; }
    if (v == "false" || v == "0") { out = false; return true; }
    return false;
}
#endif
