#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <atomic>
#include <string>

class Socket;

class ServerManager {
public:
    ServerManager();
    ~ServerManager();

    void startAllServers();
    void stopAllServers();

    void startMainServers();
    void stopMainServers();

    static bool isServerRunning();
    static void setServerRunning(bool running);

private:
    void runFrontendServer();
    void runBackendServer();
    void runControlServer();
    std::string getLocalIpAddress();

    Socket* frontendSocket;
    Socket* backendSocket;
    Socket* controlSocket;

    static std::atomic<bool> serverRunning;
    static std::atomic<bool> mainServerRunning;
};

#endif // SERVER_MANAGER_HPP
