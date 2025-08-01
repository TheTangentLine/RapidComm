#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <iostream>

class HttpService
{
public:
    HttpService(int clientSocket);
    ~HttpService();

    void handleRequest();

private:
    int clientSocket;
    void sendResponse(const std::string &response);
    std::string parseRequest();
    std::string extractRoute(const std::string &input);
    std::string handleRoute(std::string input);
    std::string getHtmlContent(const std::string &route = "/");
};

#endif