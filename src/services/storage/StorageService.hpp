#ifndef STORAGE_SERVICE_HPP
#define STORAGE_SERVICE_HPP

#include <string>
#include <vector>
#include <cstdint>

class StorageService
{
public:
    StorageService();
    explicit StorageService(const std::string& storageDirectory);
    ~StorageService();

    // Main storage operations
    bool saveFile(const std::string& filename, const std::vector<char>& fileData);
    std::pair<bool, std::string> saveFileWithVerification(const std::string& filename, const std::vector<char>& fileData);
    
    // File operations
    bool fileExists(const std::string& filename) const;
    size_t getFileSize(const std::string& filename) const;
    bool deleteFile(const std::string& filename);
    
    // Configuration
    void setStorageDirectory(const std::string& directory);
    std::string getStorageDirectory() const;
    void setMaxFileSize(size_t maxSize);
    size_t getMaxFileSize() const;

private:
    std::string storageDirectory;
    size_t maxFileSize;
    size_t chunkSize;
    bool enableVerification;
    
    // Helper functions
    bool createStorageDirectory();
    std::string getSafeFilename(const std::string& filename) const;
    std::string getFullPath(const std::string& filename) const;
    std::string getFileType(const std::string& filename) const;
    std::string getFileSizeString(size_t bytes) const;
    
    // Integrity and optimization functions
    uint32_t calculateChecksum(const std::vector<char>& data) const;
    std::string calculateFileHash(const std::vector<char>& data) const;
    std::string calculateSHA256Hash(const std::vector<char>& data) const;
    bool verifyIntegrity(const std::vector<char>& data, const std::string& expectedHash) const;
    std::pair<bool, std::string> writeFileInChunksAtomic(const std::string& filepath, const std::vector<char>& fileData);
    bool verifyWrittenFile(const std::string& filepath, const std::vector<char>& originalData);
    bool atomicFileMove(const std::string& tempPath, const std::string& finalPath);
    
    // Logging helpers
    void logInfo(const std::string& message) const;
    void logError(const std::string& message) const;
    void logSuccess(const std::string& message) const;
};

#endif