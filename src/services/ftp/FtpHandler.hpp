#ifndef FTP_SERVICE_HPP
#define FTP_SERVICE_HPP

#include <string>
#include <vector>
#include <map>
#include <cstdint>

class FtpHandler
{
public:
    FtpHandler(int clientSocket);
    FtpHandler();
    ~FtpHandler();

    void handleConnection();
    void handleFileUpload(const std::string& filename, const std::vector<char>& fileData);
    bool uploadFileToServer(const std::string& filename, const std::vector<char>& fileData);
    bool optimizedUpload(const std::string& filename, const std::vector<char>& fileData);
    void processFile(const std::string& filename, const std::vector<char>& fileData);
    void displayFileContent(const std::string& filename, const std::vector<char>& fileData);
    bool saveFile(const std::string& filename, const std::vector<char>& fileData);

private:
    int clientSocket;
    bool isServerMode;
    std::string storageDirectory;
    
    std::map<std::string, std::string> parseMessage(const std::string& message);
    void sendResponse(const std::string& message);
    std::string receiveData();
    std::string getFileType(const std::string& filename);
    void displayBinaryContent(const std::vector<char>& data, size_t maxBytes = 1024);
    std::string getFileSizeString(size_t bytes);
    bool createStorageDirectory();
    std::string getSafeFilename(const std::string& filename);
    
    // Quality optimization functions
    uint32_t calculateChecksum(const std::vector<char>& data);
    std::string calculateFileHash(const std::vector<char>& data);
    bool verifyIntegrity(const std::vector<char>& data, const std::string& expectedHash);
    bool transferChunk(int socket, const char* data, size_t size, size_t chunkSize = 8192);
    bool receiveWithVerification(int socket, std::vector<char>& buffer, size_t expectedSize);
};

#endif