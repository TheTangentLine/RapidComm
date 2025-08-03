#ifndef CLIENT_HANDLER_HPP
#define CLIENT_HANDLER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

class HttpHandler
{
public:
    HttpHandler(int clientSocket, bool isFrontend = true);
    ~HttpHandler();

    void handleRequest();

    std::string parseRequest();
    std::string extractRoute(const std::string &input);
    std::string extractMethod(const std::string &request);
    void serveFile(const std::string& path, const std::string& contentType);
    void sendJsonResponse(const std::string &json, int statusCode = 200);
    void sendErrorResponse(int statusCode, const std::string &message);
    void sendCorsResponse();

private:
    int clientSocket;
    bool isFrontend;
    
    // Basic HTTP handling
    void sendResponse(const std::string &response);
    std::string handleRoute(std::string input);
    std::string getHtmlContent(const std::string &route = "/");
    
    // File upload handling
    void handleFileUpload(const std::string &request);
    std::string parseMultipartData(const std::string &request, std::string &filename, std::vector<char> &fileData, 
                                   std::string &originalHash, std::string &originalSize, std::string &timestamp);
    std::string getBoundary(const std::string &contentType);
    std::string determineFileType(const std::string &extension);
    
    // Request parsing helpers
    std::map<std::string, std::string> parseHeaders(const std::string &request);
    std::string getRequestBody(const std::string &request);
    void parseIntegrityFields(const std::string &body, std::string &originalHash, std::string &originalSize, std::string &timestamp);
};

#endif