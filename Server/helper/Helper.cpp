#include "Helper.h"

void sleep_microseconds(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}
void sleep_milliseconds(unsigned int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
std::string StripQuotes(const std::string& s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}