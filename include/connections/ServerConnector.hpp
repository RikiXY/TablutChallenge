#pragma once

#include <string>
#include <unistd.h>

class ServerConnector{
    private:
    std::string address;
    uint16_t port;
    int clientSocket;
    
    public:
    ServerConnector(std::string address, uint16_t port);
    void connectToServer();
    void joinGame(std::string name);
    std::string receiveJson();
};