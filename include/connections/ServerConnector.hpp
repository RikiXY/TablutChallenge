#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

class ServerConnector {
private:
    int clientSocket = -1;

    /**
     * @brief Sends the whole buffer
     *
     * @param buffer The bytes to send
     * @param size How many bytes to send
     */
    void sendAllBytes(const void *buffer, size_t size);

    /**
     * @brief Reads exactly size bytes, blocking
     *
     * @param buffer Where to write the bytes
     * @param size How many bytes to read
     */
    void receiveAllBytes(void *buffer, size_t size);

public:
    /**
     * @brief Connects to the server. Throws if the connection fails
     *
     * @param address The server IPv4 address
     * @param port The port (5800 for White and 5801 for Black)
     */
    ServerConnector(const std::string &address, uint16_t port);

    /**
     * @brief Closes the socket
     */
    ~ServerConnector();
    ServerConnector(const ServerConnector &) = delete;            // Disable copy construction
    ServerConnector &operator=(const ServerConnector &) = delete; // Disable copy assignment

    /**
     * @brief Sends a message
     *
     * @param message The message to send
     */
    void send(const std::string &message);

    /**
     * @brief Waits for a message from the server. Blocking. Throws if the connection closes or the message is over 1 MiB
     *
     * @return The message received
     */
    std::string receive();
};