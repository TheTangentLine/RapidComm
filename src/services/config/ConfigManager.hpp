#ifndef CONFIG_MANAGER_HPP
#define CONFIG_MANAGER_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>

class ConfigManager
{
public:
    ConfigManager();
    explicit ConfigManager(const std::string& configFile);
    ~ConfigManager();

    // Load configuration from file
    bool loadConfig(const std::string& configFile = "config.env");
    
    // Get configuration values
    std::string getString(const std::string& key, const std::string& defaultValue = "") const;
    int getInt(const std::string& key, int defaultValue = 0) const;
    bool getBool(const std::string& key, bool defaultValue = false) const;
    size_t getSize(const std::string& key, size_t defaultValue = 0) const;
    
    // Set configuration values
    void setString(const std::string& key, const std::string& value);
    void setInt(const std::string& key, int value);
    void setBool(const std::string& key, bool value);
    void setSize(const std::string& key, size_t value);
    
    // Check if key exists
    bool hasKey(const std::string& key) const;
    
    // Get all keys
    std::map<std::string, std::string> getAllConfig() const;
    
    // Server configuration getters
    int getFrontendPort() const;
    int getBackendPort() const;
    std::string getStorageDirectory() const;
    size_t getMaxFileSize() const;
    size_t getChunkSize() const;
    bool isFileVerificationEnabled() const;
    bool isProgressTrackingEnabled() const;
    std::string getLogLevel() const;

private:
    std::map<std::string, std::string> config;
    std::string configFilePath;
    
    // Helper functions
    std::string trim(const std::string& str) const;
    bool parseLine(const std::string& line, std::string& key, std::string& value) const;
    void setDefaults();
    void logInfo(const std::string& message) const;
    void logError(const std::string& message) const;
};

#endif