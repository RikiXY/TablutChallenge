#include <iostream>
#include <string>
#include <sys/socket.h> // necessaria per definire il socket
#include <arpa/inet.h>  // necessaria per instaurare la connessione (i.e. serveAddress , connect(), recv(), ...)
#include <unistd.h>  // necessaria per 

using namespace std;

int main(int argc, char *argv[]){

    if (argc < 2) throw invalid_argument("Missing player role");

    // === FOR TESTING ===
    // Accepts only two fixed roles: white and black
    string name = string(argv[1]) == "white" ? "bianco" : (string(argv[1]) == "black" ? "nero" : throw invalid_argument("Not a valid name"));
    uint16_t port = string(argv[1]) == "white" ? 5800 : 5801;  // Ports defined in TablutCompetition/Tablut/src/.../util/Configuration.java

    // Establish the connection
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);  // [__domain] AddressFamily_INET = IPv4
                                                         // [__type] SOCK_STREAM = connection-oriented
                                                         // [__protocol] 0 = autodiscovery → TCP
    if (clientSocket == -1) throw domain_error("Creation failed");

    sockaddr_in serverAddress{};  // inserendo {} i campi numerici vengono in automatico inizializzati a 0
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);  // se necessario (diepnde dall'architettura del pc) converte il formato dei byte.
                                           // Host TO Network Short: il network byte order è big-endian (più significativo prima)
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");  // Assegna al campo s_addr della struttura in_addr (tipologia di sin_addr) l'IPv4 convertito in un intero a 32 bit (network byte order).

    int result = connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));  // C-style cast: (struct sockaddr*)&serverAddress
                                                                                                             //C++-style cast: reinterpret_cast<sockaddr*>(&serverAddress)

    if (result == -1) throw domain_error("Connection failed");
    cout<<"Connected to server!"<<endl;

    

    return 0;
}