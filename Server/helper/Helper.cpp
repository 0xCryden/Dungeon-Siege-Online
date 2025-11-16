#include "Helper.h"

void sleep_microseconds(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}
void sleep_milliseconds(unsigned int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}