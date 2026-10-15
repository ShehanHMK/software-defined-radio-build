#include <cstdint>
#include <iostream>
#include <string>

#include "include/ipc.h"

int main() {
    SDR::Ipc ipc;

    uint8_t myNodeId = 1;
    uint8_t dstNodeId = 2;
    uint16_t port = 7000;

    if (!ipc.create(myNodeId, port, "sdripc-send")) {
        return 1;
    }

    std::cout << "Ipc-sender example started...\n";

    while (true) {
        std::string msg;

        std::cout << "Enter message: ";
        std::getline(std::cin, msg);

        if (msg.empty()) {
            continue;
        }

        bool ok =
            ipc.send(dstNodeId, reinterpret_cast<const uint8_t*>(msg.data()),
                     msg.size());

        if (!ok) {
            std::cerr << "Send failed. \n";
        } else {
            std::cout << "Sent " << msg.size() << "bytes\n";
        }
    }

    return 0;
}