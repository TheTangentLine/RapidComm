#include "ServerManager.hpp"
#include "../socket/Socket.hpp"
#include "../http/HttpHandler.hpp"
#include "../ftp/FtpHandler.hpp"
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#define FRONTEND_PORT 3000
#define BACKEND_PORT 8080
#define FTP_PORT 2121

std::atomic<bool> ServerManager::serverRunning(true);

ServerManager::ServerManager() : frontendSocket(nullptr), backendSocket(nullptr), ftpSocket(nullptr) {}

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
    std::cout << "========================================" << std::endl;
    std::cout << "  HTTP-FTP File Upload Server" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Frontend: http://localhost:" << FRONTEND_PORT << std::endl;
    std::cout << "Backend:  http://localhost:" << BACKEND_PORT << std::endl;
    std::cout << "FTP:      localhost:" << FTP_PORT << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Press Ctrl+C to stop all servers" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        std::thread frontendThread(&ServerManager::runFrontendServer, this);
        std::thread backendThread(&ServerManager::runBackendServer, this);
        std::thread ftpThread(&ServerManager::runFtpServer, this);
        
        frontendThread.join();
        backendThread.join();
        ftpThread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] Error starting servers: " << e.what() << std::endl;
        throw;
    }
    
    std::cout << "[Main] All servers stopped successfully" << std::endl;
}

void ServerManager::stopAllServers() {
    std::cout << "[ServerManager] Stopping all servers..." << std::endl;
    serverRunning = false;
    
    // Close all server sockets to interrupt accept() calls
    if (frontendSocket) {
        close(frontendSocket->getServerSocket());
    }
    if (backendSocket) {
        close(backendSocket->getServerSocket());
    }
    if (ftpSocket) {
        close(ftpSocket->getServerSocket());
    }
    
    std::cout << "[ServerManager] Server sockets closed" << std::endl;
}

void ServerManager::runFrontendServer() {
    try {
        Socket frontendServer(FRONTEND_PORT);
        frontendSocket = &frontendServer;
        frontendServer.listenSocket();
        
        std::cout << "[Frontend] Server started on port " << FRONTEND_PORT << std::endl;
        
        while (serverRunning) {
            int clientSocket = accept(frontendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[Frontend] Accept failed: " << strerror(errno) << std::endl;
                }
                break; // Exit loop on error
            }
            
            if (!serverRunning) break; // Check again after accept
            
            std::cout << "[Frontend] Client connected" << std::endl;
            HttpHandler httpHandler(clientSocket, true);
            httpHandler.handleRequest();
        }
        
        frontendSocket = nullptr;
        std::cout << "[Frontend] Server stopped" << std::endl;
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
        
        std::cout << "[Backend] Server started on port " << BACKEND_PORT << std::endl;
        
        while (serverRunning) {
            int clientSocket = accept(backendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[Backend] Accept failed: " << strerror(errno) << std::endl;
                }
                break; // Exit loop on error
            }
            
            if (!serverRunning) break; // Check again after accept
            
            std::cout << "[Backend] Client connected" << std::endl;
            HttpHandler httpHandler(clientSocket, false);
            httpHandler.handleRequest();
        }
        
        backendSocket = nullptr;
        std::cout << "[Backend] Server stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Backend] Server error: " << e.what() << std::endl;
        backendSocket = nullptr;
    }
}

void ServerManager::runFtpServer() {
    try {
        Socket ftpServer(FTP_PORT);
        ftpSocket = &ftpServer;
        ftpServer.listenSocket();
        
        std::cout << "[FTP] Server started on port " << FTP_PORT << std::endl;
        
        while (serverRunning) {
            int clientSocket = accept(ftpServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[FTP] Accept failed: " << strerror(errno) << std::endl;
                }
                break; // Exit loop on error
            }
            
            if (!serverRunning) break; // Check again after accept
            
            std::cout << "[FTP] Client connected" << std::endl;
            FtpHandler ftpHandler(clientSocket);
            ftpHandler.handleConnection();
        }
        
        ftpSocket = nullptr;
        std::cout << "[FTP] Server stopped" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Server error: " << e.what() << std::endl;
        ftpSocket = nullptr;
    }
}