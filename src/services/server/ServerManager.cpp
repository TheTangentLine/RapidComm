#include "ServerManager.hpp"
#include "../socket/Socket.hpp"
#include "../http/HttpHandler.hpp"
#include "../config/ConfigManager.hpp"
#include <iostream>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <chrono>

// Colors for terminal output
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_CYAN    "\033[0;36m"
#define COLOR_RESET   "\033[0m"

#define FRONTEND_PORT 3000
#define BACKEND_PORT 8080
#define CONTROL_PORT 8081

std::atomic<bool> ServerManager::serverRunning(true);
std::atomic<bool> ServerManager::mainServerRunning(false);

std::string ServerManager::getLocalIpAddress() {
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return "127.0.0.1";
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) { // We are looking for IPv4
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == 0) {
                std::string ip(host);
                if (ip != "127.0.0.1" && ip.rfind("169.254", 0) != 0) {
                    freeifaddrs(ifaddr);
                    return ip;
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return "127.0.0.1"; // Fallback
}

ServerManager::ServerManager() : frontendSocket(nullptr), backendSocket(nullptr), controlSocket(nullptr) {}

ServerManager::~ServerManager() {
    stopAllServers();
}

void ServerManager::setServerRunning(bool running) {
    serverRunning = running;
}

bool ServerManager::isServerRunning() {
    return mainServerRunning;
}

void ServerManager::startAllServers() {
    std::thread controlThread(&ServerManager::runControlServer, this);
    controlThread.detach(); 

    while(serverRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << COLOR_GREEN << "All servers stopped ✅" << COLOR_RESET << std::endl;
}

void ServerManager::startMainServers() {
    if (mainServerRunning) {
        std::cout << COLOR_YELLOW << "Main servers are already running." << COLOR_RESET << std::endl;
        return;
    }

    mainServerRunning = true;
    std::cout << COLOR_BLUE << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "  Starting RapidComm File Upload Server" << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "========================================" << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "Frontend: http://" << getLocalIpAddress() << ":" << FRONTEND_PORT << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "Backend:  http://localhost:" << BACKEND_PORT << COLOR_RESET << std::endl;
    ConfigManager config;
    std::cout << COLOR_CYAN << "Storage:  " << config.getStorageDirectory() << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "========================================" << COLOR_RESET << std::endl;
    std::cout.flush();

    try {
        std::thread frontendThread(&ServerManager::runFrontendServer, this);
        std::thread backendThread(&ServerManager::runBackendServer, this);
        frontendThread.detach();
        backendThread.detach();
    } catch (const std::exception& e) {
        std::cerr << "[Main] Error starting main servers: " << e.what() << std::endl;
        mainServerRunning = false;
    }
}

void ServerManager::stopMainServers() {
    if (!mainServerRunning) {
        std::cout << COLOR_YELLOW << "Main servers are not running." << COLOR_RESET << std::endl;
        return;
    }
    mainServerRunning = false;

    if (frontendSocket) {
        close(frontendSocket->getServerSocket());
        frontendSocket = nullptr;
    }
    if (backendSocket) {
        close(backendSocket->getServerSocket());
        backendSocket = nullptr;
    }
    std::cout << COLOR_GREEN << "Main servers stopping..." << COLOR_RESET << std::endl;
}

void ServerManager::stopAllServers() {
    serverRunning = false;
    stopMainServers();
    if (controlSocket) {
        close(controlSocket->getServerSocket());
        controlSocket = nullptr;
    }
}

void ServerManager::runControlServer() {
    try {
        Socket controlServer(CONTROL_PORT);
        controlSocket = &controlServer;
        controlServer.listenSocket();
        
        std::cout << COLOR_GREEN << "[Control] API Server started on port " << CONTROL_PORT << COLOR_RESET << std::endl;
        
        while (serverRunning) {
            int clientSocket = accept(controlServer.getServerSocket(), nullptr, nullptr);
            if (clientSocket < 0) {
                if(serverRunning) perror("Control accept error");
                continue;
            }

            HttpHandler handler(clientSocket, false);
            std::string request = handler.parseRequest();
            std::string method = handler.extractMethod(request);
            std::string route = handler.extractRoute(request);
            
            // Required for CORS
            if (method == "OPTIONS") {
                handler.sendCorsResponse();
                close(clientSocket);
                continue;
            }

            if (method == "GET" && route == "/api/status") {
                std::string ip = getLocalIpAddress();
                std::string json = "{\"isRunning\":" + std::string(mainServerRunning ? "true" : "false") + ",\"ipAddress\":\"" + ip + "\"}";
                handler.sendJsonResponse(json);
            } else if (method == "POST" && route == "/api/start") {
                startMainServers();
                handler.sendJsonResponse("{\"status\":\"ok\",\"message\":\"Servers starting\"}");
            } else if (method == "POST" && route == "/api/stop") {
                stopMainServers();
                handler.sendJsonResponse("{\"status\":\"ok\",\"message\":\"Servers stopping\"}");
            } else {
                handler.sendErrorResponse(404, "Not Found");
            }
            close(clientSocket);
        }
    } catch (const std::exception& e) {
        std::cerr << "[Control] Server error: " << e.what() << std::endl;
        controlSocket = nullptr;
    }
}

void ServerManager::runFrontendServer() {
    try {
        Socket frontendServer(FRONTEND_PORT);
        frontendSocket = &frontendServer;
        frontendServer.listenSocket();
        
        std::cout << COLOR_GREEN << "[Frontend] Server started on port " << FRONTEND_PORT << COLOR_RESET << std::endl;
        std::cout.flush();
        
        while (mainServerRunning) {
            int clientSocket = accept(frontendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (mainServerRunning) {
                    std::cerr << "[Frontend] Accept failed: " << strerror(errno) << std::endl;
                }
                break;
            }
            
            if (!mainServerRunning) {
                close(clientSocket);
                break;
            };
            
            HttpHandler httpHandler(clientSocket, true);
            httpHandler.handleRequest();
             close(clientSocket);
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
        
        while (mainServerRunning) {
            int clientSocket = accept(backendServer.getServerSocket(), nullptr, nullptr);
            
            if (clientSocket == -1) {
                if (mainServerRunning) {
                    std::cerr << "[Backend] Accept failed: " << strerror(errno) << std::endl;
                }
                break;
            }
            
             if (!mainServerRunning) {
                close(clientSocket);
                break;
            };
            
            HttpHandler httpHandler(clientSocket, false);
            httpHandler.handleRequest();
             close(clientSocket);
        }
        
        backendSocket = nullptr;
        std::cout << COLOR_GREEN << "Backend stopped" << COLOR_RESET << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[Backend] Server error: " << e.what() << std::endl;
        backendSocket = nullptr;
    }
}
