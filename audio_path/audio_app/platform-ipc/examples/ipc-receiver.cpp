#include <array>
#include <cstdint>
#include <iostream>
#include <string>

#include "include/ipc.h"

int main() {
    SDR::Ipc ipc;

    uint8_t myNodeId = 2;
    uint16_t port = 7000;

    if (!ipc.create(myNodeId, port, "sdripc-recv")) {
        return 1;
    }

    std::cout << "Receiver started\n";

    std::array<uint8_t, 2048> buffer{};

    while (true) {
        int received = ipc.receive(buffer.data(), buffer.size());

        if (received <= 0) {
            std::cerr << "Receive failed\n";
            continue;
        }

        std::string msg(reinterpret_cast<char*>(buffer.data()), received);

        std::cout << "Received: " << msg << "\n";
    }

    return 0;
}