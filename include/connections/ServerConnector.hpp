#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

class ServerConnector {
private:
    int clientSocket = -1;
    void sendAllBytes(const void *buffer, size_t size);
    void receiveAllBytes(void *buffer, size_t size);

public:
    ServerConnector(const std::string &address, uint16_t port);
    ~ServerConnector();
    ServerConnector(const ServerConnector &) = delete;
    ServerConnector &operator=(const ServerConnector &) = delete;

    void send(const std::string &message);
    std::string receive();
};