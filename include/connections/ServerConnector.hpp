#pragma once

#include <cstdint>
#include <string>
#include <unistd.h>

class ServerConnector {
private:
    std::string address;
    uint16_t port;
    int clientSocket;
    void sendAllBytes(void *, size_t, int);
    void receiveAllBytes(void *, size_t, int);

public:
    ServerConnector(std::string address, uint16_t port);
    void connectToServer();
    void joinGame(std::string name);
    std::string receiveJson();
    void sendAction(std::string);
    void endConnection();
};