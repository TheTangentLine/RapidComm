#include <iostream>
#include <thread>
#include <csignal>
#include <atomic>
#include "socket/Socket.hpp"
#include "http/HttpService.hpp"
#include "ftp/FtpService.hpp"

// ----------------------------- Configuration -------------------------------->

#define FRONTEND_PORT 3000
#define BACKEND_PORT 8080
#define FTP_PORT 2121

// Global flag for graceful shutdown
std::atomic<bool> serverRunning(true);

// ----------------------------- Signal Handler -------------------------------->

void signalHandler(int signal) {
    std::cout << "\n[Main] Received signal " << signal << ". Shutting down servers..." << std::endl;
    serverRunning = false;
}

// ----------------------------- Server Functions ----------------------------->

// Frontend Server - Serves HTML files and static content
void runFrontendServer() {
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
            HttpService httpService(clientSocket, true);
            httpService.handleRequest();
        }
    } catch (const std::exception& e) {
        std::cerr << "[Frontend] Server error: " << e.what() << std::endl;
    }
}

// Backend Server - Handles API requests and business logic
void runBackendServer() {
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
            HttpService httpService(clientSocket, false);
            httpService.handleRequest();
        }
    } catch (const std::exception& e) {
        std::cerr << "[Backend] Server error: " << e.what() << std::endl;
    }
}

// FTP Server - Handles file storage and retrieval
void runFtpServer() {
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
            FtpService ftpService(clientSocket);
            ftpService.handleConnection();
        }
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Server error: " << e.what() << std::endl;
    }
}

// ----------------------------- Main Function -------------------------------->

int main() {
    // Set up signal handling for graceful shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
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
        // Start all three servers in separate threads
        std::thread frontendThread(runFrontendServer);
        std::thread backendThread(runBackendServer);
        std::thread ftpThread(runFtpServer);
        
        // Wait for all threads to complete
        frontendThread.join();
        backendThread.join();
        ftpThread.join();
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] Error starting servers: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "[Main] All servers stopped successfully" << std::endl;
    return 0;
}