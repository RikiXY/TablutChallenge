#include "connections/ServerConnection.hpp"

ServerConnection::ServerConnection(std::string address, uint16_t port)
    : address(address), port(port), clientSocket(-1) {}

void ServerConnection::connectToServer(){
    // Establish the connection
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);  // [__domain] AddressFamily_INET = IPv4
                                                        // [__type] SOCK_STREAM = connection-oriented
                                                        // [__protocol] 0 = autodiscovery → TCP
    if (clientSocket == -1) 
        throw std::domain_error("Creation failed");

    sockaddr_in serverAddress{};  // inserendo {} i campi numerici vengono in automatico inizializzati a 0
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);  // se necessario (diepnde dall'architettura del pc) converte il formato dei byte.
                                        // Host TO Network Short: il network byte order è big-endian (più significativo prima)
                                        // Short perché i valori possibili sono sempre o 5800 o 5801 e bastano 2 byte
    serverAddress.sin_addr.s_addr = inet_addr(address.c_str());  // Assegna al campo s_addr della struttura in_addr (tipologia di sin_addr) l'IPv4 convertito in un intero a 32 bit (network byte order).
                                                                // l'address di default è 127.0.0.1

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress)) == -1)  //C++-style cast: reinterpret_cast<sockaddr*>(&serverAddress)
                                                                                                            // C-style cast: (struct sockaddr*)&serverAddress
        throw std::domain_error("Connection failed");
}

void ServerConnection::joinGame(std::string name){
    // invio prima la dimensione del nome
    uint32_t nameSize = htonl(uint32_t(name.size()));  // uint32_t perché il protocollo del server prevede che la lunghezza sia rappresentata con esattamente 4 byte e in Network byte order
    send(clientSocket, &nameSize, sizeof(nameSize), 0);  // dichiaro il puntatore, la dimensione del puntatore, e che non ci siano altre flag
    
    // ora invio il nome
    send(clientSocket, name.data(), name.size(), 0);  // mando attraverso il socker i byte del nome e la dimensione del nome completo. non ci sono altri flag
}

std::string ServerConnection::receiveJson(){
    uint32_t jsonSize;
    size_t recieved = recv(clientSocket, &jsonSize, sizeof(jsonSize), MSG_WAITALL);

    if (recieved != sizeof(jsonSize))
        throw std::runtime_error("Failed to recieve all JSON length bytes");

    jsonSize = ntohl(jsonSize);
    std::string json(jsonSize, '\0');  // string <name>(<size>, <char>) → create a string filled with <char>
    recieved = recv(clientSocket, &json[0], json.size(), MSG_WAITALL);

    if (recieved != static_cast<size_t>(json.size()))
        throw std::runtime_error("Failed to receive JSON");

    return json;
}