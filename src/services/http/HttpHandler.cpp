#include "HttpHandler.hpp"
#include "../ftp/FtpHandler.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

// ----------------------------- Constructor --------------------------------->

HttpHandler::HttpHandler(int clientSocket, bool isFrontend) 
    : clientSocket(clientSocket), isFrontend(isFrontend) {}

// ----------------------------- Handle request ------------------------------->

void HttpHandler::handleRequest()
{
    std::string request = parseRequest();
    std::cout << "[" << (isFrontend ? "Frontend" : "Backend") << "] Request received" << std::endl;

    std::string method = extractMethod(request);
    std::string route = extractRoute(request);
    
    std::cout << "[" << (isFrontend ? "Frontend" : "Backend") << "] " << method << " " << route << std::endl;
    
    // Handle OPTIONS requests for CORS
    if (method == "OPTIONS") {
        sendCorsResponse();
        return;
    }
    
    // Frontend server - serve static files
    if (isFrontend) {
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Connection: close\r\n\r\n";
        response += handleRoute(route);
        
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    // Backend server - handle API requests
    else {
        if (method == "POST" && route == "/upload") {
            handleFileUpload(request);
        } else {
            sendErrorResponse(404, "Endpoint not found");
        }
    }
}

std::string HttpHandler::parseRequest()
{
    std::string request;
    char buffer[8192]; // Smaller chunks for streaming
    
    // First, read the headers to get Content-Length
    std::string headers;
    size_t headerEnd = std::string::npos;
    
    while (headerEnd == std::string::npos) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        
        if (bytesRead <= 0) {
            std::cerr << "[Backend] Error reading request headers!" << std::endl;
            return "";
        }
        
        request.append(buffer, bytesRead);
        headerEnd = request.find("\r\n\r\n");
        
        // Prevent infinite loop with very large headers
        if (request.size() > 1024 * 1024) { // 1MB header limit
            std::cerr << "[Backend] Headers too large!" << std::endl;
            return "";
        }
    }
    
    // Parse Content-Length from headers
    size_t contentLengthPos = request.find("Content-Length:");
    if (contentLengthPos == std::string::npos) {
        contentLengthPos = request.find("content-length:");
    }
    
    size_t totalContentLength = 0;
    if (contentLengthPos != std::string::npos) {
        size_t valueStart = request.find(":", contentLengthPos) + 1;
        size_t valueEnd = request.find("\r\n", valueStart);
        if (valueEnd != std::string::npos) {
            std::string lengthStr = request.substr(valueStart, valueEnd - valueStart);
            // Trim whitespace
            lengthStr.erase(0, lengthStr.find_first_not_of(" \t"));
            lengthStr.erase(lengthStr.find_last_not_of(" \t") + 1);
            totalContentLength = std::stoull(lengthStr);
            
            std::cout << "[Backend] Content-Length: " << totalContentLength << " bytes" << std::endl;
        }
    }
    
    // Calculate how much body we still need to read
    size_t headersSize = headerEnd + 4; // +4 for \r\n\r\n
    size_t bodyAlreadyRead = request.size() - headersSize;
    size_t bodyStillNeeded = totalContentLength - bodyAlreadyRead;
    
    // Clean HTTP processing without verbose output
    
    // Read remaining body if needed
    while (bodyStillNeeded > 0) {
        size_t chunkSize = std::min(bodyStillNeeded, sizeof(buffer) - 1);
        memset(buffer, 0, sizeof(buffer));
        
        int bytesRead = read(clientSocket, buffer, chunkSize);
        if (bytesRead <= 0) {
            std::cerr << "[Backend] Error reading request body!" << std::endl;
            break;
        }
        
        request.append(buffer, bytesRead);
        bodyStillNeeded -= bytesRead;
    }
    
    // Request processing completed
    return request;
}

// ---------------------------- Handle response ------------------------------>

std::string HttpHandler::handleRoute(std::string input)
{
    std::string content = "";
    std::string route = "";
    if (input == "/")
    {
        route = "../../src/interface/index.html";
    }
    else
    {
        route = "../../src/interface/nothingToExplore.html";
    }
    content = getHtmlContent(route);
    return content;
}

std::string HttpHandler::extractRoute(const std::string &request)
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

std::string HttpHandler::extractMethod(const std::string &request)
{
    std::stringstream ss(request);
    std::string requestLine;
    std::getline(ss, requestLine);
    
    size_t methodEnd = requestLine.find(" ");
    return requestLine.substr(0, methodEnd);
}

std::string HttpHandler::getHtmlContent(const std::string &route)
{
    std::ifstream file(route);
    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

// ---------------------------- File Upload Handling ------------------------->

void HttpHandler::handleFileUpload(const std::string &request)
{
    std::cout << "[Backend] Processing file upload..." << std::endl;
    
    std::string filename;
    std::vector<char> fileData;
    
    std::string result = parseMultipartData(request, filename, fileData);
    
    if (result == "success") {
        std::cout << "[Backend] File upload successful: " << filename << " (" << fileData.size() << " bytes)" << std::endl;
        
        // Send file to FTP server using optimized upload
        FtpHandler ftpClient; // Client mode constructor
        ftpClient.handleFileUpload(filename, fileData);
        
        sendJsonResponse("{\"status\":\"success\",\"message\":\"File uploaded successfully\",\"filename\":\"" + filename + "\"}");
    } else {
        std::cout << "[Backend] File upload failed: " << result << std::endl;
        sendErrorResponse(400, result);
    }
}

std::string HttpHandler::parseMultipartData(const std::string &request, std::string &filename, std::vector<char> &fileData)
{
    // Parse headers to get content type and boundary
    auto headers = parseHeaders(request);
    
    auto contentTypeIt = headers.find("content-type");
    if (contentTypeIt == headers.end()) {
        return "Missing Content-Type header";
    }
    
    std::string boundary = getBoundary(contentTypeIt->second);
    if (boundary.empty()) {
        return "Invalid multipart boundary";
    }
    
    // Get request body
    std::string body = getRequestBody(request);
    
    // Find file data in multipart body
    std::string boundaryDelim = "--" + boundary;
    
    // Look for file form field (try different variations)
    size_t fileStart = std::string::npos;
    std::vector<std::string> fieldPatterns = {
        "Content-Disposition: form-data; name=\"file\"",
        "content-disposition: form-data; name=\"file\"",
        "Content-Disposition: form-data; name=file",
        "content-disposition: form-data; name=file"
    };
    
    for (const auto& pattern : fieldPatterns) {
        fileStart = body.find(pattern);
        if (fileStart != std::string::npos) {
            break;
        }
    }
    
    if (fileStart == std::string::npos) {
        return "File field not found";
    }
    
    // Extract filename
    size_t filenameStart = body.find("filename=\"", fileStart);
    if (filenameStart == std::string::npos) {
        filenameStart = body.find("filename=", fileStart);
        if (filenameStart != std::string::npos) {
            filenameStart += 9; // Length of "filename="
            size_t filenameEnd = body.find_first_of(" \r\n", filenameStart);
            if (filenameEnd != std::string::npos) {
                filename = body.substr(filenameStart, filenameEnd - filenameStart);
            }
        } else {
            return "Filename not found";
        }
    } else {
        filenameStart += 10; // Length of "filename=\""
        size_t filenameEnd = body.find("\"", filenameStart);
        if (filenameEnd == std::string::npos) {
            return "Invalid filename format";
        }
        filename = body.substr(filenameStart, filenameEnd - filenameStart);
    }
    
    // Find file content start (after headers)
    size_t dataStart = body.find("\r\n\r\n", fileStart);
    if (dataStart == std::string::npos) {
        // Try alternative line endings
        dataStart = body.find("\n\n", fileStart);
        if (dataStart != std::string::npos) {
            dataStart += 2;
        } else {
            return "File data not found";
        }
    } else {
        dataStart += 4; // Skip \r\n\r\n
    }
    
    // Find file content end (next boundary) - try multiple patterns
    size_t dataEnd = std::string::npos;
    std::vector<std::string> endPatterns = {
        "\r\n--" + boundary,
        "\n--" + boundary,
        "--" + boundary
    };
    
    for (const auto& pattern : endPatterns) {
        dataEnd = body.find(pattern, dataStart);
        if (dataEnd != std::string::npos) {
            break;
        }
    }
    
    if (dataEnd == std::string::npos) {
        return "File data end not found";
    }
    
    // Extract file data
    size_t fileSize = dataEnd - dataStart;
    
    if (fileSize > 0) {
        fileData.assign(body.begin() + dataStart, body.begin() + dataEnd);
    } else {
        return "Invalid file size";
    }
    
    return "success";
}

std::string HttpHandler::getBoundary(const std::string &contentType)
{
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos) {
        return "";
    }
    
    return contentType.substr(boundaryPos + 9);
}

// ---------------------------- Helper Functions ------------------------------>

void HttpHandler::sendCorsResponse()
{
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type\r\n";
    response += "Connection: close\r\n\r\n";
    
    send(clientSocket, response.c_str(), response.length(), 0);
}

void HttpHandler::sendJsonResponse(const std::string &json, int statusCode)
{
    std::string statusText = (statusCode == 200) ? "OK" : "Error";
    
    std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " + statusText + "\r\n";
    response += "Content-Type: application/json\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Connection: close\r\n\r\n";
    response += json;
    
    send(clientSocket, response.c_str(), response.length(), 0);
}

void HttpHandler::sendErrorResponse(int statusCode, const std::string &message)
{
    std::string json = "{\"status\":\"error\",\"message\":\"" + message + "\"}";
    sendJsonResponse(json, statusCode);
}

std::map<std::string, std::string> HttpHandler::parseHeaders(const std::string &request)
{
    std::map<std::string, std::string> headers;
    std::stringstream ss(request);
    std::string line;
    
    // Skip request line
    std::getline(ss, line);
    
    // Parse headers
    while (std::getline(ss, line) && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // Convert key to lowercase and trim whitespace
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            
            // Trim leading/trailing whitespace from value
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r") + 1);
            
            headers[key] = value;
        }
    }
    
    return headers;
}

std::string HttpHandler::getRequestBody(const std::string &request)
{
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart == std::string::npos) {
        return "";
    }
    
    return request.substr(bodyStart + 4);
}

// ---------------------------- Stop listening ------------------------------>

HttpHandler::~HttpHandler()
{
    close(clientSocket);
}