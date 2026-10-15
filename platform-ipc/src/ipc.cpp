#include "include/ipc.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

namespace SDR {

Ipc::Ipc() : socketFd_(-1), nodeId_(0), port_(0) {}

Ipc::~Ipc() {
    if (socketFd_ >= 0) {
        close(socketFd_);
    }
}

std::string Ipc::makeIp(
    uint8_t nodeId) const {  // TODO(Kavindu): make this inline.
    return "10.10.10." + std::to_string(static_cast<int>(nodeId));
}

bool Ipc::create(uint8_t nodeId, uint16_t port, const std::string& tunName) {
    nodeId_ = nodeId;
    port_ = port;
    tunName_ = tunName;
    localIp_ = makeIp(nodeId_);

    if (!setupIpcInterface()) {
        std::cerr << "Failed to setup TUN interface\n";
        return false;
    }

    // Opening a socket.
    socketFd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (socketFd_ < 0) {
        perror("socket");
        return false;
    }

    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port_);

    if (inet_pton(AF_INET, localIp_.c_str(), &localAddr.sin_addr) != 1) {
        std::cerr << "Invalid local IP: " << localIp_ << "\n";
        return false;
    }

    // Bind the ipc (localAddr) to the socket.
    if (bind(socketFd_, reinterpret_cast<sockaddr*>(&localAddr),
             sizeof(localAddr)) < 0) {
        perror("bind");
        return false;
    }

    return true;
}

bool Ipc::setupIpcInterface() {
    int tunFd = open("/dev/net/tun", O_RDWR);
    if (tunFd < 0) {
        std::cerr << "Failed to open /dev/net/tun: " << std::strerror(errno)
                  << "\n";
        return false;
    }

    // Configure the TUN request structure.
    struct ifreq ifr{};
    std::memset(&ifr, 0, sizeof(ifr));

    // IFF_TUN = TUN device (Ip packets), IFF_NO_PI = no extra packet info
    // header.
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    if (!tunName_.empty()) {
        std::strncpy(ifr.ifr_name, tunName_.c_str(), IF_NAMESIZE - 1);
    }

    // Create the interface
    if (ioctl(tunFd, TUNSETIFF, (void*)&ifr) < 0) {
        std::cerr << "TUNSETIFF ioctl failed: " << std::strerror(errno) << "\n";
        return false;
    }

    tunFd_ = tunFd;

    // Open a dummy socket to configure the IP address and flags.
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create control socket: " << std::strerror(errno)
                  << std::endl;
        return false;
    }

    struct sockaddr_in addr{};
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    // Set IP Address (equivalent to `ip addr replace localIp_/24`)
    if (inet_pton(AF_INET, localIp_.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "Invalid IP address format: " << localIp_ << std::endl;
        close(sock);
        return false;
    }

    std::memcpy(&ifr.ifr_addr, &addr, sizeof(struct sockaddr));
    if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
        std::cerr << "SIOCSIFADDR failed: " << std::strerror(errno)
                  << std::endl;
        close(sock);
        return false;
    }

    // Set Subnet Mask (/24 = 255.255.255.0)
    inet_pton(AF_INET, "255.255.255.0", &addr.sin_addr);
    std::memcpy(&ifr.ifr_netmask, &addr, sizeof(struct sockaddr));
    if (ioctl(sock, SIOCSIFNETMASK, &ifr) < 0) {
        std::cerr << "SIOCSIFNETMASK failed: " << std::strerror(errno)
                  << std::endl;
        close(sock);
        return false;
    }

    // 3. Bring the interface UP (equivalent to `ip link set dev ... up`)
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        std::cerr << "SIOCGIFFLAGS failed: " << std::strerror(errno)
                  << std::endl;
        close(sock);
        return false;
    }

    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
        std::cerr << "SIOCSIFFLAGS failed: " << std::strerror(errno)
                  << std::endl;
        close(sock);
        return false;
    }

    close(sock);  // Socket configuration complete
    return true;
}

bool Ipc::send(uint8_t dstNodeId, const uint8_t* data, size_t length) {
    if (socketFd_ < 0 || data == nullptr || length == 0) {
        return false;
    }

    std::string dstIp = makeIp(dstNodeId);

    sockaddr_in dstAddr{};
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_port = htons(port_);

    if (inet_pton(AF_INET, dstIp.c_str(), &dstAddr.sin_addr) != 1) {
        std::cerr << "Invalid destination IP: " << dstIp << "\n";
        return false;
    }

    ssize_t sent =
        sendto(socketFd_, data, length, 0,
               reinterpret_cast<sockaddr*>(&dstAddr), sizeof(dstAddr));

    if (sent < 0) {
        perror("sendto");
        return false;
    }

    return static_cast<size_t>(sent) == length;
}

int Ipc::receive(uint8_t* buffer, size_t bufferSize) {
    if (socketFd_ < 0 || buffer == nullptr || bufferSize == 0) {
        return -1;
    }

    ssize_t received =
        recvfrom(socketFd_, buffer, bufferSize, 0, nullptr, nullptr);

    if (received < 0) {
        perror("recvfrom");
        return -1;
    }

    return static_cast<int>(received);
}

std::string Ipc::getlocalIp() const {
    return localIp_;
}

}  // namespace SDR