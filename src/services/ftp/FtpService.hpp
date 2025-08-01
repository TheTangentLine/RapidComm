#ifndef FTP_SERVICE_HPP
#define FTP_SERVICE_HPP

#include <string>
#include <vector>
#include <map>

class FtpService
{
public:
    // Constructor for server mode (accepts client connections)
    FtpService(int clientSocket);
    
    // Constructor for client mode (connects to FTP server)
    FtpService();
    
    ~FtpService();

    // ----------------------------- Server Methods ------------------------------>
    
    // Handle incoming client connection (for FTP server)
    void handleConnection();
    
    // ----------------------------- Client Methods ------------------------------>
    
    // Connect to FTP server and upload file (for backend use)
    bool uploadFileToServer(const std::string& filename, const std::vector<char>& fileData);
    
    // ----------------------------- File Processing ------------------------------>
    
    // Process and store uploaded file
    void processFile(const std::string& filename, const std::vector<char>& fileData);
    
    // Display file content in console
    void displayFileContent(const std::string& filename, const std::vector<char>& fileData);
    
    // Save file to storage directory
    bool saveFile(const std::string& filename, const std::vector<char>& fileData);

private:
    int clientSocket;
    bool isServerMode;
    std::string storageDirectory;
    
    // ----------------------------- Protocol Handling --------------------------->
    
    // Parse incoming FTP protocol messages
    std::map<std::string, std::string> parseMessage(const std::string& message);
    
    // Send response to client
    void sendResponse(const std::string& message);
    
    // Receive data from client
    std::string receiveData();
    
    // ----------------------------- File Operations ------------------------------>
    
    // Helper function to determine file type based on extension
    std::string getFileType(const std::string& filename);
    
    // Helper function to safely display binary content
    void displayBinaryContent(const std::vector<char>& data, size_t maxBytes = 1024);
    
    // Get file size in human readable format
    std::string getFileSizeString(size_t bytes);
    
    // Create storage directory if it doesn't exist
    bool createStorageDirectory();
    
    // Get safe filename (sanitize input)
    std::string getSafeFilename(const std::string& filename);
};

#endif