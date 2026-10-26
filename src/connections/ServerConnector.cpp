#include "connections/ServerConnector.hpp"

#include <sys/socket.h> // necessaria per definire il socket
#include <arpa/inet.h>  // necessaria per instaurare la connessione (i.e. serveAddress , connect(), recv(), ...)
#include <stdexcept>

ServerConnector::ServerConnector(std::string address, uint16_t port)
    : address(address), port(port), clientSocket(-1) {}

void ServerConnector::connectToServer(){
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

void ServerConnector::sendAllBytes(void* buffer, size_t size, int flag){  // dato che il buffer può essere il puntatore di un intero come di una stringa devo mettere void* che indica un puntatore generico
                                                                          // con un puntatore generico il compilatore non sa come fare la somma tra puntatori (i.e. int* +1 sposta di 4 byte, char* +1 sposta di 1 byte)
                                                                          // quindi dato che io voglio sempre spostarmi byte per byte faccio un cast a char* in send()
    ssize_t sent = 0;
    size_t totalSent = 0;
    while(totalSent < size){
        sent = send(clientSocket, static_cast<char*>(buffer) + totalSent, size - totalSent, flag);  // the buffer pointer shifts and the size must be decreased
        if (sent > 0)
            totalSent += static_cast<size_t>(sent);
        else if (sent == 0) 
            throw std::runtime_error("Connection closed");
        else if (sent == -1)
            throw std::runtime_error("Failed to send data");
    }
}

void ServerConnector::joinGame(std::string name){  // Send <name> to server
    std::string fixedName = "\"" + name + "\"";
    
    // invio prima la dimensione del nome
    uint32_t nameSize = htonl(uint32_t(fixedName.size()));  // uint32_t perché il protocollo del server prevede che la lunghezza sia rappresentata con esattamente 4 byte e in Network byte order
    
    sendAllBytes(&nameSize, sizeof(nameSize), MSG_NOSIGNAL);  // dichiaro il puntatore, la dimensione del puntatore, e altre flag
                                                              // MSG_NOSIGNAL flag blocks the exception in the case socket is closed, giving -1 as result
                                                                
    
    // ora invio il nome
    sendAllBytes(fixedName.data(), fixedName.size(), MSG_NOSIGNAL);  // mando attraverso il socker i byte del nome e la dimensione del nome completo. non ci sono altri flag
}
 
void ServerConnector::receiveAllBytes(void* buffer, size_t size, int flag){
    ssize_t received = 0;
    size_t totalReceived = 0;
    while(totalReceived < size){
        received = recv(clientSocket, static_cast<char*>(buffer) + totalReceived, size - totalReceived, flag); // chiamata bloccante
        if (received > 0)
            totalReceived += static_cast<size_t>(received);
        else if (received == 0) 
            throw std::runtime_error("Connection closed");
        else if (received == -1)
            throw std::runtime_error("Failed to receive data");
    } 
}

std::string ServerConnector::receiveJson(){  // Receive from server a raw string and returns it as string containing the JSON
    
    // receive json dimension
    uint32_t jsonSize;
    receiveAllBytes(&jsonSize, sizeof(jsonSize), MSG_WAITALL);
    // ! bisognerebbe verificare le dimensioni del JSON per evitare che mi esploda il programma

    jsonSize = ntohl(jsonSize);
    std::string json(jsonSize, '\0');  // string <name>(<size>, <char>) → create a string filled with <char>
    receiveAllBytes(&json[0], json.size(), MSG_WAITALL);

    // ! bisognerebbe validare il JSON

    return json;
}

void ServerConnector::sendAction(std::string action){
    uint32_t actionSize = htonl(uint32_t(action.size()));
    sendAllBytes(&actionSize, sizeof(actionSize), MSG_NOSIGNAL);
    sendAllBytes(action.data(), action.size(), MSG_NOSIGNAL);
}

void ServerConnector::endConnection(){
    close(clientSocket);
    clientSocket = -1;
}