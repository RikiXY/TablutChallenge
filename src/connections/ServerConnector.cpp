#include "connections/ServerConnector.hpp"
#include <arpa/inet.h> // IP address conversion and network byte-order utilities
#include <cerrno>
#include <chrono>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/socket.h>
#include <system_error>
#include <thread>
#include <unistd.h>

ServerConnector::ServerConnector(const std::string &address, uint16_t port) {
    sockaddr_in serverAddress{}; // Zero-initialize all structure members
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port); // Convert the port to network byte order (big-endian)

    const std::string host = address == "localhost" ? "127.0.0.1" : address;
    if (inet_pton(AF_INET, host.c_str(), &serverAddress.sin_addr) != 1) { // Convert the IPv4 address from text to binary (network byte order)
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        addrinfo *found = nullptr;
        if (getaddrinfo(host.c_str(), nullptr, &hints, &found) != 0 || found == nullptr)
            throw std::invalid_argument("Not a valid address or host name: " + address);
        serverAddress.sin_addr = reinterpret_cast<sockaddr_in *>(found->ai_addr)->sin_addr;
        freeaddrinfo(found);
    }

    constexpr int attempts = 50;
    for (int attempt = 1;; attempt++) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0); // AF_INET: IPv4 address family
                                                        // 0: Automatically select the default protocol (TCP)
                                                        // SOCK_STREAM: Connection-oriented byte stream
        if (clientSocket == -1)
            throw std::system_error(errno, std::system_category(), "Socket creation failed");

        if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == 0)
            break;
        const int error = errno;
        close(clientSocket);
        clientSocket = -1;
        if (attempt == attempts)
            throw std::system_error(error, std::system_category(), "Connection to " + address + " failed");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    const int noDelay = 1;
    setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, &noDelay, sizeof(noDelay));
}

ServerConnector::~ServerConnector() {
    close(clientSocket);
}

void ServerConnector::sendAllBytes(const void *buffer, size_t size) {
    size_t totalSent = 0;
    while (totalSent < size) {
        const ssize_t sent = ::send(clientSocket, static_cast<const char *>(buffer) + totalSent, size - totalSent, MSG_NOSIGNAL);
        if (sent == -1)
            throw std::system_error(errno, std::system_category(), "Failed to send data");
        totalSent += static_cast<size_t>(sent);
    }
}

void ServerConnector::receiveAllBytes(void *buffer, size_t size) {
    size_t totalReceived = 0;
    while (totalReceived < size) {
        const ssize_t received = ::recv(clientSocket, static_cast<char *>(buffer) + totalReceived, size - totalReceived, MSG_WAITALL); // chiamata bloccante
        if (received == 0)
            throw std::runtime_error("Connection closed by the server");
        if (received == -1)
            throw std::system_error(errno, std::system_category(), "Failed to receive data");
        totalReceived += static_cast<size_t>(received);
    }
}

void ServerConnector::send(const std::string &message) {
    const uint32_t size = htonl(uint32_t(message.size()));
    sendAllBytes(&size, sizeof(size));
    sendAllBytes(message.data(), message.size());
}

std::string ServerConnector::receive() {
    uint32_t size;
    receiveAllBytes(&size, sizeof(size));
    size = ntohl(size);

    constexpr uint32_t maxSize = 1 << 20; // 2^20 bytes = 1 MiB
    if (size > maxSize)
        throw std::runtime_error("Message too large: " + std::to_string(size) + " bytes");

    std::string message(size, '\0');
    receiveAllBytes(message.data(), message.size());
    return message;
}