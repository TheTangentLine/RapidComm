#include "ServerManager.hpp"
#include "../socket/Socket.hpp"
#include "../http/HttpHandler.hpp"
#include "../ftp/FtpHandler.hpp"
#include <iostream>
#include <thread>

#define FRONTEND_PORT 3000
#define BACKEND_PORT 8080
#define FTP_PORT 2121

std::atomic<bool> ServerManager::serverRunning(true);

ServerManager::ServerManager() {}

ServerManager::~ServerManager() {}

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
    serverRunning = false;
}

void ServerManager::runFrontendServer() {
    try {
        Socket frontendServer(FRONTEND_PORT);
        frontendServer.listenSocket();
        
        std::cout << "[Frontend] Server started on port " << FRONTEND_PORT << std::endl;
        
        while (serverRunning) {
            int clientSocket = accept(frontendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[Frontend] Failed to accept client connection!" << std::endl;
                }
                continue;
            }
            
            std::cout << "[Frontend] Client connected" << std::endl;
            HttpHandler httpHandler(clientSocket, true);
            httpHandler.handleRequest();
        }
    } catch (const std::exception& e) {
        std::cerr << "[Frontend] Server error: " << e.what() << std::endl;
    }
}

void ServerManager::runBackendServer() {
    try {
        Socket backendServer(BACKEND_PORT);
        backendServer.listenSocket();
        
        std::cout << "[Backend] Server started on port " << BACKEND_PORT << std::endl;
        
        while (serverRunning) {
            int clientSocket = accept(backendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[Backend] Failed to accept client connection!" << std::endl;
                }
                continue;
            }
            
            std::cout << "[Backend] Client connected" << std::endl;
            HttpHandler httpHandler(clientSocket, false);
            httpHandler.handleRequest();
        }
    } catch (const std::exception& e) {
        std::cerr << "[Backend] Server error: " << e.what() << std::endl;
    }
}

void ServerManager::runFtpServer() {
    try {
        Socket ftpServer(FTP_PORT);
        ftpServer.listenSocket();
        
        std::cout << "[FTP] Server started on port " << FTP_PORT << std::endl;
        
        while (serverRunning) {
            int clientSocket = accept(ftpServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (serverRunning) {
                    std::cerr << "[FTP] Failed to accept client connection!" << std::endl;
                }
                continue;
            }
            
            std::cout << "[FTP] Client connected" << std::endl;
            FtpHandler ftpHandler(clientSocket);
            ftpHandler.handleConnection();
        }
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Server error: " << e.what() << std::endl;
    }
}