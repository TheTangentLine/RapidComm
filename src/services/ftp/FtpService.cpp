#include "FtpService.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

// ----------------------------- Constructors -------------------------------->

// Server mode constructor (accepts connections)
FtpService::FtpService(int clientSocket) 
    : clientSocket(clientSocket), isServerMode(true), storageDirectory("uploads/")
{
    std::cout << "[FTP] Server mode initialized" << std::endl;
    createStorageDirectory();
}

// Client mode constructor (connects to server)
FtpService::FtpService() 
    : clientSocket(-1), isServerMode(false), storageDirectory("uploads/")
{
    std::cout << "[FTP] Client mode initialized" << std::endl;
}

// ----------------------------- Destructor ----------------------------------->

FtpService::~FtpService() 
{
    if (clientSocket != -1) {
        close(clientSocket);
    }
    std::cout << "[FTP] Service destroyed" << std::endl;
}

// ----------------------------- Server Methods ------------------------------>

void FtpService::handleConnection()
{
    std::cout << "[FTP] Handling incoming connection..." << std::endl;
    std::cout << "[FTP] Server mode: " << (isServerMode ? "enabled" : "disabled") << std::endl;
    
    try {
        // Send welcome message
        sendResponse("FTP_READY");
        
        while (true) {
            std::string request = receiveData();
            if (request.empty()) {
                std::cout << "[FTP] Client disconnected" << std::endl;
                break;
            }
            
            auto parsedMessage = parseMessage(request);
            std::string command = parsedMessage["command"];
            
            if (command == "UPLOAD") {
                std::string filename = parsedMessage["filename"];
                std::string fileSizeStr = parsedMessage["size"];
                
                if (filename.empty() || fileSizeStr.empty()) {
                    sendResponse("ERROR Invalid upload parameters");
                    continue;
                }
                
                size_t fileSize = std::stoull(fileSizeStr);
                std::cout << "[FTP] Receiving file: " << filename << " (" << getFileSizeString(fileSize) << ")" << std::endl;
                
                // Send ready signal
                sendResponse("READY_FOR_DATA");
                
                // Receive file data
                std::vector<char> fileData(fileSize);
                size_t totalReceived = 0;
                
                while (totalReceived < fileSize) {
                    ssize_t received = recv(clientSocket, fileData.data() + totalReceived, 
                                          fileSize - totalReceived, 0);
                    if (received <= 0) {
                        std::cerr << "[FTP] Error receiving file data" << std::endl;
                        sendResponse("ERROR Failed to receive file data");
                        break;
                    }
                    totalReceived += received;
                }
                
                if (totalReceived == fileSize) {
                    fileData.resize(totalReceived);
                    processFile(filename, fileData);
                    
                    if (saveFile(filename, fileData)) {
                        sendResponse("SUCCESS File uploaded successfully");
                        std::cout << "[FTP] File saved successfully: " << filename << std::endl;
                    } else {
                        sendResponse("ERROR Failed to save file");
                        std::cerr << "[FTP] Failed to save file: " << filename << std::endl;
                    }
                } else {
                    sendResponse("ERROR Incomplete file transfer");
                }
                
            } else if (command == "QUIT") {
                sendResponse("GOODBYE");
                break;
            } else {
                sendResponse("ERROR Unknown command");
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Connection error: " << e.what() << std::endl;
    }
}

// ----------------------------- Client Methods ------------------------------>

bool FtpService::uploadFileToServer(const std::string& filename, const std::vector<char>& fileData)
{
    std::cout << "[FTP] Connecting to FTP server..." << std::endl;
    
    // Create socket for FTP connection
    int ftpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ftpSocket == -1) {
        std::cerr << "[FTP] Failed to create socket" << std::endl;
        return false;
    }
    
    // Configure server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2121); // FTP_PORT
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Connect to FTP server
    if (connect(ftpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "[FTP] Failed to connect to FTP server" << std::endl;
        close(ftpSocket);
        return false;
    }
    
    std::cout << "[FTP] Connected to FTP server" << std::endl;
    
    try {
        // Receive welcome message
        char buffer[1024];
        recv(ftpSocket, buffer, sizeof(buffer), 0);
        
        // Send upload command
        std::string uploadCmd = "UPLOAD|filename=" + filename + "|size=" + std::to_string(fileData.size()) + "\n";
        send(ftpSocket, uploadCmd.c_str(), uploadCmd.length(), 0);
        
        // Wait for ready signal
        memset(buffer, 0, sizeof(buffer));
        recv(ftpSocket, buffer, sizeof(buffer), 0);
        std::string response(buffer);
        
        if (response.find("READY_FOR_DATA") != std::string::npos) {
            // Send file data
            size_t totalSent = 0;
            while (totalSent < fileData.size()) {
                ssize_t sent = send(ftpSocket, fileData.data() + totalSent, 
                                  fileData.size() - totalSent, 0);
                if (sent <= 0) {
                    std::cerr << "[FTP] Error sending file data" << std::endl;
                    close(ftpSocket);
                    return false;
                }
                totalSent += sent;
            }
            
            // Receive final response
            memset(buffer, 0, sizeof(buffer));
            recv(ftpSocket, buffer, sizeof(buffer), 0);
            std::string finalResponse(buffer);
            
            // Send quit command
            std::string quitCmd = "QUIT\n";
            send(ftpSocket, quitCmd.c_str(), quitCmd.length(), 0);
            
            close(ftpSocket);
            
            bool success = finalResponse.find("SUCCESS") != std::string::npos;
            std::cout << "[FTP] Upload " << (success ? "successful" : "failed") << std::endl;
            return success;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Upload error: " << e.what() << std::endl;
    }
    
    close(ftpSocket);
    return false;
}

// ----------------------------- File Processing ------------------------------>

void FtpService::processFile(const std::string& filename, const std::vector<char>& fileData)
{
    std::cout << "\n========== FTP FILE PROCESSING ==========" << std::endl;
    std::cout << "[FTP] Processing file: " << filename << std::endl;
    std::cout << "[FTP] File size: " << getFileSizeString(fileData.size()) << std::endl;
    std::cout << "[FTP] File type: " << getFileType(filename) << std::endl;
    
    // Display file content
    displayFileContent(filename, fileData);
    
    std::cout << "=========================================" << std::endl;
}

void FtpService::displayFileContent(const std::string& filename, const std::vector<char>& fileData)
{
    std::string fileType = getFileType(filename);
    
    std::cout << "\n[FTP] File Content Preview:" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
    
    if (fileType == "text" || fileType == "html" || fileType == "css" || fileType == "javascript" || fileType == "json") {
        // Display text content directly
        std::string content(fileData.begin(), fileData.end());
        
        // Limit display to first 2000 characters for readability
        if (content.length() > 2000) {
            std::cout << content.substr(0, 2000) << std::endl;
            std::cout << "\n... [Content truncated - showing first 2000 characters] ..." << std::endl;
        } else {
            std::cout << content << std::endl;
        }
    } else {
        // For binary files, display in hex format with limited bytes
        std::cout << "[Binary file detected - showing hex dump]" << std::endl;
        displayBinaryContent(fileData);
    }
    
    std::cout << "-------------------------------------------" << std::endl;
}

bool FtpService::saveFile(const std::string& filename, const std::vector<char>& fileData)
{
    try {
        std::string safeFilename = getSafeFilename(filename);
        std::string filepath = storageDirectory + safeFilename;
        
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "[FTP] Failed to create file: " << filepath << std::endl;
            return false;
        }
        
        file.write(fileData.data(), fileData.size());
        file.close();
        
        std::cout << "[FTP] File saved to: " << filepath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Error saving file: " << e.what() << std::endl;
        return false;
    }
}

// ----------------------------- Protocol Handling --------------------------->

std::map<std::string, std::string> FtpService::parseMessage(const std::string& message)
{
    std::map<std::string, std::string> result;
    std::istringstream iss(message);
    std::string line;
    
    if (std::getline(iss, line)) {
        // Remove trailing newline/carriage return
        line.erase(line.find_last_not_of("\r\n") + 1);
        
        std::istringstream lineStream(line);
        std::string token;
        
        // First token is the command
        if (std::getline(lineStream, token, '|')) {
            result["command"] = token;
        }
        
        // Parse key=value pairs
        while (std::getline(lineStream, token, '|')) {
            size_t equalPos = token.find('=');
            if (equalPos != std::string::npos) {
                std::string key = token.substr(0, equalPos);
                std::string value = token.substr(equalPos + 1);
                result[key] = value;
            }
        }
    }
    
    return result;
}

void FtpService::sendResponse(const std::string& message)
{
    std::string response = message + "\n";
    send(clientSocket, response.c_str(), response.length(), 0);
}

std::string FtpService::receiveData()
{
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    
    ssize_t received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        return "";
    }
    
    return std::string(buffer, received);
}

// ----------------------------- Helper Functions ----------------------------->

std::string FtpService::getFileType(const std::string& filename)
{
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "unknown";
    }
    
    std::string extension = filename.substr(dotPos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    // Text file types
    if (extension == "txt" || extension == "log" || extension == "md" || extension == "readme") {
        return "text";
    }
    // Web file types
    else if (extension == "html" || extension == "htm") {
        return "html";
    }
    else if (extension == "css") {
        return "css";
    }
    else if (extension == "js") {
        return "javascript";
    }
    else if (extension == "json") {
        return "json";
    }
    // Image file types
    else if (extension == "jpg" || extension == "jpeg" || extension == "png" || extension == "gif" || extension == "bmp") {
        return "image";
    }
    // Document file types
    else if (extension == "pdf" || extension == "doc" || extension == "docx") {
        return "document";
    }
    // Archive file types
    else if (extension == "zip" || extension == "rar" || extension == "tar" || extension == "gz") {
        return "archive";
    }
    
    return "binary";
}

void FtpService::displayBinaryContent(const std::vector<char>& data, size_t maxBytes)
{
    size_t bytesToShow = std::min(data.size(), maxBytes);
    
    for (size_t i = 0; i < bytesToShow; i += 16) {
        // Show address
        std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
        
        // Show hex values
        for (size_t j = 0; j < 16 && (i + j) < bytesToShow; ++j) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << (static_cast<unsigned char>(data[i + j])) << " ";
        }
        
        // Padding for incomplete lines
        for (size_t j = bytesToShow - i; j < 16; ++j) {
            std::cout << "   ";
        }
        
        // Show ASCII representation
        std::cout << " |";
        for (size_t j = 0; j < 16 && (i + j) < bytesToShow; ++j) {
            char c = data[i + j];
            std::cout << (std::isprint(c) ? c : '.');
        }
        std::cout << "|" << std::endl;
    }
    
    if (data.size() > maxBytes) {
        std::cout << "\n... [Binary content truncated - showing first " << maxBytes << " bytes] ..." << std::endl;
    }
    
    // Reset cout formatting
    std::cout << std::dec;
}

std::string FtpService::getFileSizeString(size_t bytes)
{
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024 && unitIndex < 3) {
        size /= 1024;
        unitIndex++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
}

bool FtpService::createStorageDirectory()
{
    try {
        if (!std::filesystem::exists(storageDirectory)) {
            std::filesystem::create_directories(storageDirectory);
            std::cout << "[FTP] Created storage directory: " << storageDirectory << std::endl;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Failed to create storage directory: " << e.what() << std::endl;
        return false;
    }
}

std::string FtpService::getSafeFilename(const std::string& filename)
{
    std::string safeFilename = filename;
    
    // Remove path separators and dangerous characters
    std::replace(safeFilename.begin(), safeFilename.end(), '/', '_');
    std::replace(safeFilename.begin(), safeFilename.end(), '\\', '_');
    std::replace(safeFilename.begin(), safeFilename.end(), '.', '_');
    
    // Add extension back if it was there originally
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos) {
        std::string extension = filename.substr(lastDot);
        safeFilename += extension;
    }
    
    return safeFilename;
}