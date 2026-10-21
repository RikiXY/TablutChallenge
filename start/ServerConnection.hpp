#pragma once

#include <string>
#include <unistd.h>
#include <sys/socket.h> // necessaria per definire il socket
#include <arpa/inet.h>  // necessaria per instaurare la connessione (i.e. serveAddress , connect(), recv(), ...)
#include <stdexcept>

class ServerConnection{
    private:
    std::string address;
    uint16_t port;
    int clientSocket;
    
    public:
    ServerConnection(std::string address, uint16_t port);
    void connect_to_server();
    void join_game(std::string name);
    std::string receive_json();
};