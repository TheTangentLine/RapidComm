#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <atomic>

class Socket; // Forward declaration

class ServerManager
{
public:
    ServerManager();
    ~ServerManager();
    
    void startAllServers();
    void stopAllServers();
    
    static void setServerRunning(bool running);
    static bool isServerRunning();

private:
    static std::atomic<bool> serverRunning;
    
    // Store socket pointers to close them during shutdown
    Socket* frontendSocket;
    Socket* backendSocket;
    
    void runFrontendServer();
    void runBackendServer();
};

#endif