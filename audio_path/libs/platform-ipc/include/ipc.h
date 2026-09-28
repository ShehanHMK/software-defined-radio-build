#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace SDR {

// struct IpcConfig {
//     // Node Id becomes the last byte of the TUN IP.
//     // Node Id = 1 -> 10.0.10.1
//     // Node Id = 2 -> 10.0.10.2

//     uint8_t node_id;

//     // Name of the tun interface.
//     std::string tun_name = "sdripc0";

//     // Ip prefix
//     std::string ip_prefix = "10.0.10.";

//     // TUN subnet prefix length
//     uint8_t prefixLength = 24;

//     uint16_t port;
// };

class Ipc {
public:
    Ipc();
    ~Ipc();

    bool create(uint8_t nodeId, uint16_t port,
                const std::string& tunName = "sdripc0");

    bool send(uint8_t dstNodeId, const uint8_t* data, size_t length);

    int receive(uint8_t* buffer, size_t bufferSize);

    std::string getlocalIp() const;

private:
    int socketFd_;
    uint8_t nodeId_;
    uint16_t port_;
    std::string tunName_;
    int tunFd_;
    std::string localIp_;

    std::string makeIp(uint8_t nodeId) const;
    bool setupIpcInterface();
};

}  // namespace SDR