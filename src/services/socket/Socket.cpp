#include "Socket.hpp"
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// ---------------------------------- Constructor ----------------------------------->

Socket::Socket(int port) : port(port)
{
    // Creating a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        std::cerr << "Failed to create socket!" << std::endl;
        exit(1);
    }

    // Setup server address information
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(this->port);
}

// ---------------------------------- Getter ----------------------------------->

int Socket::getServerSocket()
{
    return this->serverSocket;
}

// ---------------------------------- Start listening ----------------------------------->

void Socket::listenSocket()
{
    if (bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Bind failed!" << std::endl;
        closeSocket();
        exit(1);
    }

    if (listen(serverSocket, 3) == -1)
    {
        std::cerr << "Listen failed!" << std::endl;
        closeSocket();
        exit(1);
    }
}

// ---------------------------------- Stop listening ----------------------------------->

void Socket::closeSocket()
{
    close(this->serverSocket);
}

Socket::~Socket()
{
    closeSocket();
}