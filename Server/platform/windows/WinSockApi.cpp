#include "WinSockApi.h"
#include "../Common.h"

WinSockApi::WinSockApi() {
    WSADATA data{};
    const int result = WSAStartup(MAKEWORD(2, 2), &data);
    if (result != 0) {
        throw std::runtime_error("WSAStartup failed: " + to_string(result));
    }
}

WinSockApi::~WinSockApi() noexcept {
    WSACleanup();
}
