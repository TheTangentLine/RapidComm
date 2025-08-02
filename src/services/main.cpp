#include <iostream>
#include <csignal>
#include "server/ServerManager.hpp"

// Colors for terminal output
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_RESET   "\033[0m"

ServerManager* serverManager = nullptr;

void signalHandler(int signal) {
    std::cout << "\n" << COLOR_YELLOW << "[Main] Received signal " << signal << ". Shutting down servers..." << COLOR_RESET << std::endl;
    std::cout.flush();
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
        std::cerr << COLOR_RED << "[Main] Error starting servers: " << e.what() << COLOR_RESET << std::endl;
        return 1;
    }
    
    return 0;
}
