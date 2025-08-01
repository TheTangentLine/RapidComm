#include <iostream>
#include "socket/Socket.hpp"
#include "http/HttpService.hpp"

#define PORT 8080

int main()
{
    // Initialize socket and server address
    Socket server(PORT);

    // Bind socket
    server.listenSocket();

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    // Infinite loops for receiving incoming requests
    while (true)
    {
        // server.getServerSocket() would return 3 as file descriptor for Socket.
        // clientSocket may have a value of 4 as 0, 1, 2 has been there.
        int clientSocket = accept(server.getServerSocket(), nullptr, nullptr);

        if (clientSocket == -1)
        {
            std::cerr << "Failed to accept client connection!" << std::endl;
            continue;
        }

        HttpService HttpService(clientSocket);
        HttpService.handleRequest();
    }

    return 0;
}