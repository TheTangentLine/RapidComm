#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <iostream>
#include <vector>
#include <map>

class HttpHandler
{
public:
    HttpHandler(int clientSocket, bool isFrontend = true);
    ~HttpHandler();

    void handleRequest();

private:
    int clientSocket;
    bool isFrontend;
    
    // Basic HTTP handling
    void sendResponse(const std::string &response);
    std::string parseRequest();
    std::string extractRoute(const std::string &input);
    std::string extractMethod(const std::string &request);
    std::string handleRoute(std::string input);
    std::string getHtmlContent(const std::string &route = "/");
    
    // File upload handling
    void handleFileUpload(const std::string &request);
    std::string parseMultipartData(const std::string &request, std::string &filename, std::vector<char> &fileData);
    std::string getBoundary(const std::string &contentType);
    
    // CORS and response helpers
    void sendCorsResponse();
    void sendJsonResponse(const std::string &json, int statusCode = 200);
    void sendErrorResponse(int statusCode, const std::string &message);
    
    // Request parsing helpers
    std::map<std::string, std::string> parseHeaders(const std::string &request);
    std::string getRequestBody(const std::string &request);
};

#endif