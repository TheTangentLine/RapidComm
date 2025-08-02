#include "FtpHandler.hpp"
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
#include <chrono>
#include <functional>

// ----------------------------- Constructor --------------------------------->

FtpHandler::FtpHandler(int clientSocket) 
    : clientSocket(clientSocket), isServerMode(true), storageDirectory("uploads/")
{
    std::cout << "[FTP] Server mode initialized" << std::endl;
    createStorageDirectory();
}


FtpHandler::FtpHandler() 
    : clientSocket(-1), isServerMode(false), storageDirectory("uploads/")
{
    std::cout << "[FTP] Client mode initialized" << std::endl;
}

// ----------------------------- Destructor ----------------------------------->

FtpHandler::~FtpHandler() 
{
    if (clientSocket != -1) {
        close(clientSocket);
    }
    std::cout << "[FTP] Service destroyed" << std::endl;
}

// ----------------------------- Connection Handling -------------------------->

void FtpHandler::handleConnection()
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

// ----------------------------- Upload Handling ------------------------------>

void FtpHandler::handleFileUpload(const std::string& filename, const std::vector<char>& fileData)
{
    std::cout << "[FTP] Handling optimized file upload: " << filename << " (" << fileData.size() << " bytes)" << std::endl;
    
    // Calculate file integrity hash before processing
    std::string fileHash = calculateFileHash(fileData);
    std::cout << "[FTP] File integrity hash: " << fileHash << std::endl;
    
    // Use optimized upload method
    bool uploaded = optimizedUpload(filename, fileData);
    
    if (uploaded) {
        // Verify integrity after upload
        std::cout << "[FTP] File upload completed successfully: " << filename << std::endl;
        std::cout << "[FTP] Quality maintained - Hash verified" << std::endl;
        sendResponse("UPLOAD_SUCCESS File uploaded with integrity verification");
    } else {
        std::cout << "[FTP] Optimized file upload failed: " << filename << std::endl;
        sendResponse("UPLOAD_ERROR Failed to upload with quality assurance");
    }
}

// ----------------------------- Client Upload Methods ----------------------->

bool FtpHandler::uploadFileToServer(const std::string& filename, const std::vector<char>& fileData)
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

void FtpHandler::processFile(const std::string& filename, const std::vector<char>& fileData)
{
    std::cout << "\n========== FTP FILE PROCESSING ==========" << std::endl;
    std::cout << "[FTP] Processing file: " << filename << std::endl;
    std::cout << "[FTP] File size: " << getFileSizeString(fileData.size()) << std::endl;
    std::cout << "[FTP] File type: " << getFileType(filename) << std::endl;
    
    // Display file content
    displayFileContent(filename, fileData);
    
    std::cout << "=========================================" << std::endl;
}

void FtpHandler::displayFileContent(const std::string& filename, const std::vector<char>& fileData)
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

bool FtpHandler::saveFile(const std::string& filename, const std::vector<char>& fileData)
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

std::map<std::string, std::string> FtpHandler::parseMessage(const std::string& message)
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

void FtpHandler::sendResponse(const std::string& message)
{
    std::string response = message + "\n";
    send(clientSocket, response.c_str(), response.length(), 0);
}

std::string FtpHandler::receiveData()
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

std::string FtpHandler::getFileType(const std::string& filename)
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

void FtpHandler::displayBinaryContent(const std::vector<char>& data, size_t maxBytes)
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

std::string FtpHandler::getFileSizeString(size_t bytes)
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

bool FtpHandler::createStorageDirectory()
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

std::string FtpHandler::getSafeFilename(const std::string& filename)
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

// ----------------------------- Quality Optimization Functions ---------------->

uint32_t FtpHandler::calculateChecksum(const std::vector<char>& data)
{
    uint32_t checksum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        checksum += static_cast<uint8_t>(data[i]);
        checksum = (checksum << 1) | (checksum >> 31); // Rotate left by 1
    }
    return checksum;
}

std::string FtpHandler::calculateFileHash(const std::vector<char>& data)
{
    // Create a robust hash using std::hash with multiple passes
    std::hash<std::string> hasher;
    std::string dataStr(data.begin(), data.end());
    
    // Multi-pass hashing for better distribution
    size_t hash1 = hasher(dataStr);
    size_t hash2 = hasher(dataStr + std::to_string(data.size()));
    uint32_t checksum = calculateChecksum(data);
    
    std::ostringstream oss;
    oss << std::hex << hash1 << hash2 << checksum;
    return oss.str();
}

bool FtpHandler::verifyIntegrity(const std::vector<char>& data, const std::string& expectedHash)
{
    std::string actualHash = calculateFileHash(data);
    bool isValid = (actualHash == expectedHash);
    
    std::cout << "[FTP] Integrity check - Expected: " << expectedHash.substr(0, 16) << "..." << std::endl;
    std::cout << "[FTP] Integrity check - Actual:   " << actualHash.substr(0, 16) << "..." << std::endl;
    std::cout << "[FTP] Integrity status: " << (isValid ? "VALID" : "CORRUPTED") << std::endl;
    
    return isValid;
}

bool FtpHandler::transferChunk(int socket, const char* data, size_t size, size_t chunkSize)
{
    size_t totalSent = 0;
    const int maxRetries = 3;
    
    while (totalSent < size) {
        size_t remainingSize = size - totalSent;
        size_t currentChunkSize = std::min(chunkSize, remainingSize);
        
        int retries = 0;
        ssize_t sent = 0;
        
        while (retries < maxRetries) {
            sent = send(socket, data + totalSent, currentChunkSize, 0);
            
            if (sent > 0) {
                totalSent += sent;
                break;
            } else if (sent == -1) {
                retries++;
                std::cerr << "[FTP] Chunk transfer retry " << retries << "/" << maxRetries << std::endl;
                usleep(100000); // 100ms delay before retry
            }
        }
        
        if (sent <= 0) {
            std::cerr << "[FTP] Failed to transfer chunk after " << maxRetries << " retries" << std::endl;
            return false;
        }
        
        // Progress feedback
        if (size > 1024 * 1024) { // Show progress for files > 1MB
            double progress = (double)totalSent / size * 100.0;
            if ((int)progress % 10 == 0) {
                std::cout << "[FTP] Transfer progress: " << (int)progress << "%" << std::endl;
            }
        }
    }
    
    return true;
}

bool FtpHandler::receiveWithVerification(int socket, std::vector<char>& buffer, size_t expectedSize)
{
    buffer.clear();
    buffer.reserve(expectedSize);
    
    size_t totalReceived = 0;
    char tempBuffer[8192];
    const int maxRetries = 3;
    
    while (totalReceived < expectedSize) {
        size_t remainingSize = expectedSize - totalReceived;
        size_t currentChunkSize = std::min(sizeof(tempBuffer), remainingSize);
        
        int retries = 0;
        ssize_t received = 0;
        
        while (retries < maxRetries) {
            received = recv(socket, tempBuffer, currentChunkSize, 0);
            
            if (received > 0) {
                buffer.insert(buffer.end(), tempBuffer, tempBuffer + received);
                totalReceived += received;
                break;
            } else if (received == -1) {
                retries++;
                std::cerr << "[FTP] Receive retry " << retries << "/" << maxRetries << std::endl;
                usleep(100000); // 100ms delay before retry
            }
        }
        
        if (received <= 0) {
            std::cerr << "[FTP] Failed to receive chunk after " << maxRetries << " retries" << std::endl;
            return false;
        }
    }
    
    return (totalReceived == expectedSize);
}

bool FtpHandler::optimizedUpload(const std::string& filename, const std::vector<char>& fileData)
{
    std::cout << "[FTP] Starting optimized upload for: " << filename << std::endl;
    
    // Calculate original file hash for integrity verification
    std::string originalHash = calculateFileHash(fileData);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        // Process the file with integrity tracking
        processFile(filename, fileData);
        
        // Optimized save with verification
        std::string safeFilename = getSafeFilename(filename);
        std::string fullPath = storageDirectory + safeFilename;
        
        // Create output stream with binary mode for perfect quality preservation
        std::ofstream file(fullPath, std::ios::binary | std::ios::out);
        if (!file.is_open()) {
            std::cerr << "[FTP] Failed to create file: " << fullPath << std::endl;
            return false;
        }
        
        // Write in optimized chunks
        const size_t CHUNK_SIZE = 65536; // 64KB chunks for optimal performance
        size_t totalWritten = 0;
        
        while (totalWritten < fileData.size()) {
            size_t remainingSize = fileData.size() - totalWritten;
            size_t currentChunkSize = std::min(CHUNK_SIZE, remainingSize);
            
            file.write(fileData.data() + totalWritten, currentChunkSize);
            
            if (file.fail()) {
                std::cerr << "[FTP] Write error at position " << totalWritten << std::endl;
                file.close();
                return false;
            }
            
            totalWritten += currentChunkSize;
            
            // Flush after each chunk to ensure data integrity
            file.flush();
        }
        
        file.close();
        
        // Verify file was written correctly by reading it back
        std::ifstream verifyFile(fullPath, std::ios::binary);
        if (!verifyFile.is_open()) {
            std::cerr << "[FTP] Failed to open file for verification" << std::endl;
            return false;
        }
        
        std::vector<char> verifyData((std::istreambuf_iterator<char>(verifyFile)),
                                     std::istreambuf_iterator<char>());
        verifyFile.close();
        
        // Verify integrity
        bool integrityValid = verifyIntegrity(verifyData, originalHash);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        if (integrityValid) {
            std::cout << "[FTP] Optimized upload completed successfully in " << duration.count() << "ms" << std::endl;
            std::cout << "[FTP] File quality maintained - byte-perfect transfer" << std::endl;
            std::cout << "[FTP] File saved to: " << fullPath << std::endl;
            return true;
        } else {
            std::cerr << "[FTP] File integrity verification failed!" << std::endl;
            // Remove corrupted file
            std::filesystem::remove(fullPath);
            return false;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[FTP] Optimized upload error: " << e.what() << std::endl;
        return false;
    }
}