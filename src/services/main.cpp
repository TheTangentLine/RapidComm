#include <iostream>
#include <csignal>
#include "server/ServerManager.hpp"

ServerManager* serverManager = nullptr;

void signalHandler(int signal) {
    std::cout << "\n[Main] Received signal " << signal << ". Shutting down servers..." << std::endl;
    if (serverManager) {
        serverManager->stopAllServers();
    }
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        ServerManager manager;
        serverManager = &manager;
        
        manager.startAllServers();
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] Error starting servers: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}