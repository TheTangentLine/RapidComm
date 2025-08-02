#include "ServerManager.hpp"
#include "../socket/Socket.hpp"
#include "../http/HttpHandler.hpp"
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

// Colors for terminal output
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_CYAN    "\033[0;36m"
#define COLOR_RESET   "\033[0m"

#define FRONTEND_PORT 3000
#define BACKEND_PORT 8080

std::atomic<bool> ServerManager::serverRunning(true);

ServerManager::ServerManager() : frontendSocket(nullptr), backendSocket(nullptr) {}

ServerManager::~ServerManager() {
    stopAllServers();
}

void ServerManager::setServerRunning(bool running) {
    serverRunning = running;
}

bool ServerManager::isServerRunning() {
    return serverRunning;
}

void ServerManager::startAllServers() {
    std::cout << COLOR_BLUE << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "  RapidComm File Upload Server" << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "Frontend: http://localhost:" << FRONTEND_PORT << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "Backend:  http://localhost:" << BACKEND_PORT << COLOR_RESET << std::endl;
    std::cout << COLOR_CYAN << "Storage:  ./uploads/" << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_YELLOW << "Press Ctrl+C to stop all servers" << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "========================================" << COLOR_RESET << std::endl;
    std::cout.flush();
    
    try {
        std::thread frontendThread(&ServerManager::runFrontendServer, this);
        std::thread backendThread(&ServerManager::runBackendServer, this);
        
        frontendThread.join();
        backendThread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] Error starting servers: " << e.what() << std::endl;
        throw;
    }
    
    std::cout << COLOR_GREEN << "All servers stopped ✅" << COLOR_RESET << std::endl;
}

void ServerManager::stopAllServers() {
    serverRunning = false;
    
    // Close all server sockets to interrupt accept() calls
    if (frontendSocket) {
        close(frontendSocket->getServerSocket());
    }
    if (backendSocket) {
        close(backendSocket->getServerSocket());
    }
}

void ServerManager::runFrontendServer() {
    try {
        Socket frontendServer(FRONTEND_PORT);
        frontendSocket = &frontendServer;
        frontendServer.listenSocket();
        
        std::cout << COLOR_GREEN << "[Frontend] Server started on port " << FRONTEND_PORT << COLOR_RESET << std::endl;
        std::cout.flush();
        
        while (serverRunning) {
            int clientSocket = accept(frontendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[Frontend] Accept failed: " << strerror(errno) << std::endl;
                }
                break; // Exit loop on error
            }
            
            if (!serverRunning) break; // Check again after accept
            
            HttpHandler httpHandler(clientSocket, true);
            httpHandler.handleRequest();
        }
        
        frontendSocket = nullptr;
        std::cout << COLOR_GREEN << "Frontend stopped" << COLOR_RESET << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Frontend] Server error: " << e.what() << std::endl;
        frontendSocket = nullptr;
    }
}

void ServerManager::runBackendServer() {
    try {
        Socket backendServer(BACKEND_PORT);
        backendSocket = &backendServer;
        backendServer.listenSocket();
        
        std::cout << COLOR_GREEN << "[Backend] Server started on port " << BACKEND_PORT << COLOR_RESET << std::endl;
        std::cout.flush();
        
        while (serverRunning) {
            int clientSocket = accept(backendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[Backend] Accept failed: " << strerror(errno) << std::endl;
                }
                break; // Exit loop on error
            }
            
            if (!serverRunning) break; // Check again after accept
            
            HttpHandler httpHandler(clientSocket, false);
            httpHandler.handleRequest();
        }
        
        backendSocket = nullptr;
        std::cout << COLOR_GREEN << "Backend stopped" << COLOR_RESET << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Backend] Server error: " << e.what() << std::endl;
        backendSocket = nullptr;
    }
}

