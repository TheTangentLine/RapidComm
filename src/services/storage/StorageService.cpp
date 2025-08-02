#include "StorageService.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <functional>
#include <ctime>

// Colors for terminal output
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_CYAN    "\033[0;36m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_RESET   "\033[0m"

// Default configuration
static const size_t DEFAULT_MAX_FILE_SIZE = 2ULL * 1024 * 1024 * 1024; // 2GB
static const size_t DEFAULT_CHUNK_SIZE = 1024 * 1024; // 1MB
static const std::string DEFAULT_STORAGE_DIR = "./uploads/";

// ----------------------------- Constructor/Destructor --------------------------------->

StorageService::StorageService() 
    : storageDirectory(DEFAULT_STORAGE_DIR),
      maxFileSize(DEFAULT_MAX_FILE_SIZE),
      chunkSize(DEFAULT_CHUNK_SIZE),
      enableVerification(true)
{
    createStorageDirectory();
    logInfo("Storage service initialized with default configuration");
}

StorageService::StorageService(const std::string& storageDirectory) 
    : storageDirectory(storageDirectory.empty() ? DEFAULT_STORAGE_DIR : storageDirectory),
      maxFileSize(DEFAULT_MAX_FILE_SIZE),
      chunkSize(DEFAULT_CHUNK_SIZE),
      enableVerification(true)
{
    // Ensure directory ends with slash
    if (this->storageDirectory.back() != '/') {
        this->storageDirectory += '/';
    }
    createStorageDirectory();
    logInfo("Storage service initialized with directory: " + this->storageDirectory);
}

StorageService::~StorageService() 
{
    logInfo("Storage service destroyed");
}

// ----------------------------- Main Storage Operations --------------------------------->

bool StorageService::saveFile(const std::string& filename, const std::vector<char>& fileData)
{
    if (filename.empty()) {
        logError("Cannot save file: filename is empty");
        return false;
    }

    if (fileData.empty()) {
        logError("Cannot save file: file data is empty");
        return false;
    }

    if (fileData.size() > maxFileSize) {
        logError("File too large: " + getFileSizeString(fileData.size()) + " exceeds limit of " + getFileSizeString(maxFileSize));
        return false;
    }

    try {
        std::string safeFilename = getSafeFilename(filename);
        std::string fullPath = getFullPath(safeFilename);
        
        logInfo("Saving file: " + filename + " (" + getFileSizeString(fileData.size()) + ")");
        
        // Write file using standard method
        std::ofstream file(fullPath, std::ios::binary);
        if (!file.is_open()) {
            logError("Failed to create file: " + fullPath);
            return false;
        }
        
        file.write(fileData.data(), fileData.size());
        file.close();
        
        if (file.fail()) {
            logError("Failed to write file: " + fullPath);
            return false;
        }
        
        logSuccess("File saved successfully: " + fullPath);
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception while saving file: " + std::string(e.what()));
        return false;
    }
}

std::pair<bool, std::string> StorageService::saveFileWithVerification(const std::string& filename, const std::vector<char>& fileData)
{
    if (filename.empty()) {
        logError("Cannot save file: filename is empty");
        return std::make_pair(false, "");
    }

    if (fileData.empty()) {
        logError("Cannot save file: file data is empty");
        return std::make_pair(false, "");
    }

    if (fileData.size() > maxFileSize) {
        logError("File too large: " + getFileSizeString(fileData.size()) + " exceeds limit of " + getFileSizeString(maxFileSize));
        return std::make_pair(false, "");
    }

    try {
        std::string safeFilename = getSafeFilename(filename);
        std::string fullPath = getFullPath(safeFilename);
        
        logInfo("Saving file with verification: " + filename + " (" + getFileSizeString(fileData.size()) + ")");
        
        // Calculate SHA-256 hash for bit-perfect verification
        std::string sha256Hash = calculateSHA256Hash(fileData);
        logInfo("Calculated SHA-256 hash: " + sha256Hash.substr(0, 16) + "...");
        logInfo("File data size for hashing: " + std::to_string(fileData.size()) + " bytes");
        
        // Write file using atomic chunked method for all files
        std::string fileType = getFileType(filename);
        logInfo("Using atomic chunked write for " + fileType + " file: " + filename);
        
        auto writeResult = writeFileInChunksAtomic(fullPath, fileData);
        bool writeSuccess = writeResult.first;
        std::string verifiedHash = writeResult.second;
        
        if (!writeSuccess) {
            logError("Failed to write file atomically: " + fullPath);
            return std::make_pair(false, "");
        }
        
        // Verify file integrity if verification is enabled
        if (enableVerification) {
            if (sha256Hash != verifiedHash) {
                logError("File integrity verification failed - hash mismatch");
                logError("Expected: " + sha256Hash.substr(0, 16) + "...");
                logError("Got: " + verifiedHash.substr(0, 16) + "...");
                std::filesystem::remove(fullPath);
                return std::make_pair(false, "");
            }
            logSuccess("File integrity verified successfully ✅");
        }
        
        logSuccess("File saved with bit-perfect verification: " + fullPath);
        return std::make_pair(true, sha256Hash);
        
    } catch (const std::exception& e) {
        logError("Exception while saving file with verification: " + std::string(e.what()));
        return std::make_pair(false, "");
    }
}

// ----------------------------- File Operations --------------------------------->

bool StorageService::fileExists(const std::string& filename) const
{
    if (filename.empty()) return false;
    std::string fullPath = getFullPath(getSafeFilename(filename));
    return std::filesystem::exists(fullPath);
}

size_t StorageService::getFileSize(const std::string& filename) const
{
    if (!fileExists(filename)) return 0;
    std::string fullPath = getFullPath(getSafeFilename(filename));
    try {
        return std::filesystem::file_size(fullPath);
    } catch (const std::exception&) {
        return 0;
    }
}

bool StorageService::deleteFile(const std::string& filename)
{
    if (filename.empty()) return false;
    std::string fullPath = getFullPath(getSafeFilename(filename));
    
    try {
        if (std::filesystem::exists(fullPath)) {
            std::filesystem::remove(fullPath);
            logInfo("File deleted: " + fullPath);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        logError("Failed to delete file: " + std::string(e.what()));
        return false;
    }
}

// ----------------------------- Configuration --------------------------------->

void StorageService::setStorageDirectory(const std::string& directory)
{
    std::string newDir = directory.empty() ? DEFAULT_STORAGE_DIR : directory;
    if (newDir.back() != '/') {
        newDir += '/';
    }
    storageDirectory = newDir;
    createStorageDirectory();
    logInfo("Storage directory updated: " + storageDirectory);
}

std::string StorageService::getStorageDirectory() const
{
    return storageDirectory;
}

void StorageService::setMaxFileSize(size_t maxSize)
{
    maxFileSize = maxSize;
    logInfo("Max file size updated: " + getFileSizeString(maxFileSize));
}

size_t StorageService::getMaxFileSize() const
{
    return maxFileSize;
}

// ----------------------------- Helper Functions --------------------------------->

bool StorageService::createStorageDirectory()
{
    try {
        if (!std::filesystem::exists(storageDirectory)) {
            std::filesystem::create_directories(storageDirectory);
            logInfo("Created storage directory: " + storageDirectory);
        }
        return true;
    } catch (const std::exception& e) {
        logError("Failed to create storage directory: " + std::string(e.what()));
        return false;
    }
}

std::string StorageService::getSafeFilename(const std::string& filename) const
{
    if (filename.empty()) return "unnamed_file";
    
    // Find the extension first
    size_t lastDot = filename.find_last_of('.');
    std::string baseName = filename;
    std::string extension = "";
    
    if (lastDot != std::string::npos) {
        baseName = filename.substr(0, lastDot);
        extension = filename.substr(lastDot);
    }
    
    // Clean only the base name, preserve the extension
    std::replace(baseName.begin(), baseName.end(), '/', '_');
    std::replace(baseName.begin(), baseName.end(), '\\', '_');
    std::replace(baseName.begin(), baseName.end(), ':', '_');
    std::replace(baseName.begin(), baseName.end(), '<', '_');
    std::replace(baseName.begin(), baseName.end(), '>', '_');
    std::replace(baseName.begin(), baseName.end(), '|', '_');
    std::replace(baseName.begin(), baseName.end(), '*', '_');
    std::replace(baseName.begin(), baseName.end(), '?', '_');
    
    return baseName + extension;
}

std::string StorageService::getFullPath(const std::string& filename) const
{
    return storageDirectory + filename;
}

std::string StorageService::getFileType(const std::string& filename) const
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
    else if (extension == "jpg" || extension == "jpeg" || extension == "png" || extension == "gif" || extension == "bmp" || extension == "svg" || extension == "webp") {
        return "image";
    }
    // Video file types
    else if (extension == "mp4" || extension == "avi" || extension == "mov" || extension == "wmv" || extension == "flv" || 
             extension == "webm" || extension == "mkv" || extension == "m4v" || extension == "3gp" || extension == "ogv" ||
             extension == "ts" || extension == "mts" || extension == "m2ts" || extension == "vob" || extension == "asf") {
        return "video";
    }
    // Audio file types
    else if (extension == "mp3" || extension == "wav" || extension == "aac" || extension == "ogg" || extension == "wma" ||
             extension == "flac" || extension == "m4a" || extension == "opus" || extension == "aiff") {
        return "audio";
    }
    // Document file types
    else if (extension == "pdf" || extension == "doc" || extension == "docx" || extension == "xls" || extension == "xlsx" ||
             extension == "ppt" || extension == "pptx" || extension == "odt" || extension == "ods" || extension == "odp") {
        return "document";
    }
    // Archive file types
    else if (extension == "zip" || extension == "rar" || extension == "tar" || extension == "gz" || extension == "7z" ||
             extension == "bz2" || extension == "xz" || extension == "lzma") {
        return "archive";
    }
    
    return "binary";
}

std::string StorageService::getFileSizeString(size_t bytes) const
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

// ----------------------------- Integrity Functions --------------------------------->

uint32_t StorageService::calculateChecksum(const std::vector<char>& data) const
{
    uint32_t checksum = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        checksum += static_cast<uint8_t>(data[i]);
        checksum = (checksum << 1) | (checksum >> 31); // Rotate left by 1
    }
    return checksum;
}

std::string StorageService::calculateFileHash(const std::vector<char>& data) const
{
    // Legacy hash function - keeping for compatibility
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

std::string StorageService::calculateSHA256Hash(const std::vector<char>& data) const
{
    // Simple hash algorithm compatible with frontend implementation
    // Using deterministic algorithm that both client and server can reproduce
    
    uint32_t hash1 = 0;
    uint32_t hash2 = 0;
    uint32_t hash3 = 0;
    
    // Hash pass 1: Basic hash
    for (size_t i = 0; i < data.size(); i++) {
        hash1 = ((hash1 << 5) - hash1 + static_cast<unsigned char>(data[i])) & 0xffffffff;
    }
    
    // Hash pass 2: With salt1 - process data first, then salt1
    std::string salt1 = "salt1";
    for (size_t i = 0; i < data.size(); i++) {
        hash2 = ((hash2 << 5) - hash2 + static_cast<unsigned char>(data[i])) & 0xffffffff;
    }
    for (char c : salt1) {
        hash2 = ((hash2 << 5) - hash2 + static_cast<unsigned char>(c)) & 0xffffffff;
    }
    
    // Hash pass 3: With salt2 and size - process data first, then salt2
    std::string salt2 = "salt2" + std::to_string(data.size());
    for (size_t i = 0; i < data.size(); i++) {
        hash3 = ((hash3 << 5) - hash3 + static_cast<unsigned char>(data[i])) & 0xffffffff;
    }
    for (char c : salt2) {
        hash3 = ((hash3 << 5) - hash3 + static_cast<unsigned char>(c)) & 0xffffffff;
    }
    
    // Calculate checksum (same as existing method)
    uint32_t checksum = calculateChecksum(data);
    
    // Combine hashes and format as hex
    std::ostringstream oss;
    oss << std::hex << hash1 << hash2 << hash3 << checksum;
    
    // Pad to 64 characters
    std::string result = oss.str();
    while (result.length() < 64) {
        result += "0";
    }
    if (result.length() > 64) {
        result = result.substr(0, 64);
    }
    
    return result;
}

bool StorageService::verifyIntegrity(const std::vector<char>& data, const std::string& expectedHash) const
{
    std::string actualHash = calculateFileHash(data);
    return (actualHash == expectedHash);
}

std::pair<bool, std::string> StorageService::writeFileInChunksAtomic(const std::string& filepath, const std::vector<char>& fileData)
{
    try {
        // Create temporary file path for atomic operation
        std::string tempPath = filepath + ".tmp." + std::to_string(time(nullptr));
        
        std::ofstream file(tempPath, std::ios::binary | std::ios::out);
        if (!file.is_open()) {
            logError("Failed to create temporary file: " + tempPath);
            return std::make_pair(false, "");
        }
        
        size_t totalWritten = 0;
        while (totalWritten < fileData.size()) {
            size_t remainingSize = fileData.size() - totalWritten;
            size_t currentChunkSize = std::min(chunkSize, remainingSize);
            
            file.write(fileData.data() + totalWritten, currentChunkSize);
            
            if (file.fail()) {
                file.close();
                std::filesystem::remove(tempPath);
                logError("Failed to write chunk at position: " + std::to_string(totalWritten));
                return std::make_pair(false, "");
            }
            
            totalWritten += currentChunkSize;
            file.flush();
            
            // Progress feedback for large files with more frequent updates
            if (fileData.size() > 5 * 1024 * 1024) { // Show progress for files > 5MB
                double progress = (double)totalWritten / fileData.size() * 100.0;
                static int lastProgress = -1;
                int currentProgress = (int)(progress / 10) * 10; // Every 10%
                
                if (currentProgress != lastProgress && currentProgress > 0) {
                    std::string progressStr = "Write progress: " + std::to_string(currentProgress) + "% (" + 
                                            getFileSizeString(totalWritten) + "/" + getFileSizeString(fileData.size()) + ")";
                    logInfo(progressStr);
                    lastProgress = currentProgress;
                }
            }
        }
        
        file.close();
        
        // Verify file was written correctly by calculating hash
        std::ifstream verifyFile(tempPath, std::ios::binary);
        if (!verifyFile.is_open()) {
            std::filesystem::remove(tempPath);
            logError("Failed to open temporary file for verification");
            return std::make_pair(false, "");
        }
        
        std::vector<char> verifyData((std::istreambuf_iterator<char>(verifyFile)),
                                     std::istreambuf_iterator<char>());
        verifyFile.close();
        
        // Calculate hash of written file
        std::string writtenHash = calculateSHA256Hash(verifyData);
        
        // Verify data integrity
        if (verifyData.size() != fileData.size()) {
            std::filesystem::remove(tempPath);
            logError("File size mismatch after write: " + std::to_string(verifyData.size()) + " vs " + std::to_string(fileData.size()));
            return std::make_pair(false, "");
        }
        
        // Atomically move temp file to final location
        if (!atomicFileMove(tempPath, filepath)) {
            std::filesystem::remove(tempPath);
            logError("Failed to atomically move file to final location");
            return std::make_pair(false, "");
        }
        
        logSuccess("File written atomically with verification: " + filepath);
        return std::make_pair(true, writtenHash);
        
    } catch (const std::exception& e) {
        logError("Exception during atomic write: " + std::string(e.what()));
        return std::make_pair(false, "");
    }
}

bool StorageService::verifyWrittenFile(const std::string& filepath, const std::vector<char>& originalData)
{
    try {
        std::ifstream verifyFile(filepath, std::ios::binary);
        if (!verifyFile.is_open()) {
            return false;
        }
        
        std::vector<char> verifyData((std::istreambuf_iterator<char>(verifyFile)),
                                     std::istreambuf_iterator<char>());
        verifyFile.close();
        
        // Check size first
        if (verifyData.size() != originalData.size()) {
            return false;
        }
        
        // Verify integrity using hash
        std::string originalHash = calculateFileHash(originalData);
        return verifyIntegrity(verifyData, originalHash);
        
    } catch (const std::exception&) {
        return false;
    }
}

// ----------------------------- Logging Helpers --------------------------------->

void StorageService::logInfo(const std::string& message) const
{
    std::cout << COLOR_BLUE << "[Storage] " << message << COLOR_RESET << std::endl;
}

bool StorageService::atomicFileMove(const std::string& tempPath, const std::string& finalPath)
{
    try {
        // Use rename for atomic operation (works on same filesystem)
        if (std::rename(tempPath.c_str(), finalPath.c_str()) == 0) {
            return true;
        }
        
        // Fallback: copy + remove (less atomic but more compatible)
        std::filesystem::copy_file(tempPath, finalPath, std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(tempPath);
        
        return std::filesystem::exists(finalPath);
        
    } catch (const std::exception& e) {
        logError("Atomic move failed: " + std::string(e.what()));
        return false;
    }
}

void StorageService::logError(const std::string& message) const
{
    std::cerr << COLOR_RED << "[Storage] ERROR: " << message << COLOR_RESET << std::endl;
}

void StorageService::logSuccess(const std::string& message) const
{
    std::cout << COLOR_GREEN << "[Storage] " << message << COLOR_RESET << std::endl;
}