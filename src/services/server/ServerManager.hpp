#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <atomic>

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
    
    void runFrontendServer();
    void runBackendServer();
    void runFtpServer();
};

#endif