#include "HttpService.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

// ----------------------------- Constructor --------------------------------->

HttpService::HttpService(int clientSocket) : clientSocket(clientSocket) {}

// ----------------------------- Handle request ------------------------------->

void HttpService::handleRequest()
{
    std::string request = parseRequest();
    std::cout << "Request received:\n"
              << request << std::endl;

    sendResponse(request);
}

std::string HttpService::parseRequest()
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0)
    {
        std::cerr << "Error reading request!" << std::endl;
        return "";
    }

    return std::string(buffer);
}

// ---------------------------- Handle response ------------------------------>

void HttpService::sendResponse(const std::string &request)
{
    std::string route = extractRoute(request);

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Connection: close\r\n\r\n";
    response += handleRoute(route);

    send(clientSocket, response.c_str(), response.length(), 0);
}

std::string HttpService::handleRoute(std::string input = "/")
{
    std::string content = "";
    if (input == "/")
    {
        const std::string route = "../interface/index.html";
        content = getHtmlContent(route);
    }
    else
    {
        content = "<html><body><h1>There is nothing to explore here</h1></body></html>";
    }
    return content;
}

std::string HttpService::extractRoute(const std::string &request)
{
    std::stringstream ss(request);

    std::string requestLine;
    std::getline(ss, requestLine);

    size_t methodEnd = requestLine.find(" ");
    size_t routeStart = methodEnd + 1;
    size_t routeEnd = requestLine.find(" ", routeStart);

    std::string route = requestLine.substr(routeStart, routeEnd - routeStart);

    return route;
}

std::string HttpService::getHtmlContent(const std::string &route)
{
    std::ifstream file(route);
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

// ---------------------------- Stop listening ------------------------------>

HttpService::~HttpService()
{
    close(clientSocket);
}