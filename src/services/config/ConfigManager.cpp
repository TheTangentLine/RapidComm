#include "ConfigManager.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>

// Colors for terminal output
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_RESET   "\033[0m"

// ----------------------------- Constructor/Destructor --------------------------------->

ConfigManager::ConfigManager() : configFilePath("../../config.env")
{
    setDefaults();
    loadConfig(configFilePath);
}

ConfigManager::ConfigManager(const std::string& configFile) : configFilePath(configFile)
{
    setDefaults();
    loadConfig(configFile);
}

ConfigManager::~ConfigManager() 
{
    // No cleanup needed
}

// ----------------------------- Load Configuration --------------------------------->

bool ConfigManager::loadConfig(const std::string& configFile)
{
    if (!configFile.empty()) {
        configFilePath = configFile;
    }
    
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        logError("Could not open config file: " + configFilePath + ". Using defaults.");
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Skip empty lines and comments
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            continue;
        }
        
        std::string key, value;
        if (parseLine(trimmedLine, key, value)) {
            config[key] = value;
        } else {
            logError("Invalid config line " + std::to_string(lineNumber) + ": " + line);
        }
    }
    
    file.close();
    logInfo("Configuration loaded from: " + configFilePath);
    return true;
}

// ----------------------------- Get Configuration Values --------------------------------->

std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) const
{
    auto it = config.find(key);
    return (it != config.end()) ? it->second : defaultValue;
}

int ConfigManager::getInt(const std::string& key, int defaultValue) const
{
    std::string value = getString(key);
    if (value.empty()) return defaultValue;
    
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        logError("Invalid integer value for key '" + key + "': " + value);
        return defaultValue;
    }
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const
{
    std::string value = getString(key);
    if (value.empty()) return defaultValue;
    
    // Convert to lowercase for comparison
    std::string lowerValue = value;
    std::transform(lowerValue.begin(), lowerValue.end(), lowerValue.begin(), ::tolower);
    
    if (lowerValue == "true" || lowerValue == "yes" || lowerValue == "1" || lowerValue == "on") {
        return true;
    } else if (lowerValue == "false" || lowerValue == "no" || lowerValue == "0" || lowerValue == "off") {
        return false;
    } else {
        logError("Invalid boolean value for key '" + key + "': " + value);
        return defaultValue;
    }
}

size_t ConfigManager::getSize(const std::string& key, size_t defaultValue) const
{
    std::string value = getString(key);
    if (value.empty()) return defaultValue;
    
    try {
        return std::stoull(value);
    } catch (const std::exception&) {
        logError("Invalid size value for key '" + key + "': " + value);
        return defaultValue;
    }
}

// ----------------------------- Set Configuration Values --------------------------------->

void ConfigManager::setString(const std::string& key, const std::string& value)
{
    config[key] = value;
}

void ConfigManager::setInt(const std::string& key, int value)
{
    config[key] = std::to_string(value);
}

void ConfigManager::setBool(const std::string& key, bool value)
{
    config[key] = value ? "true" : "false";
}

void ConfigManager::setSize(const std::string& key, size_t value)
{
    config[key] = std::to_string(value);
}

// ----------------------------- Utility Functions --------------------------------->

bool ConfigManager::hasKey(const std::string& key) const
{
    return config.find(key) != config.end();
}

std::map<std::string, std::string> ConfigManager::getAllConfig() const
{
    return config;
}

// ----------------------------- Server Configuration Getters --------------------------------->

int ConfigManager::getFrontendPort() const
{
    return getInt("FRONTEND_PORT", 3000);
}

int ConfigManager::getBackendPort() const
{
    return getInt("BACKEND_PORT", 8080);
}

std::string ConfigManager::getStorageDirectory() const
{
    std::string dir = getString("STORAGE_DIRECTORY", "./uploads/");
    // Ensure directory ends with slash
    if (!dir.empty() && dir.back() != '/') {
        dir += '/';
    }
    return dir;
}

size_t ConfigManager::getMaxFileSize() const
{
    return getSize("STORAGE_MAX_FILE_SIZE", 104857600); // 100MB default
}

size_t ConfigManager::getChunkSize() const
{
    return getSize("STORAGE_CHUNK_SIZE", 65536); // 64KB default
}

bool ConfigManager::isFileVerificationEnabled() const
{
    return getBool("ENABLE_FILE_VERIFICATION", true);
}

bool ConfigManager::isProgressTrackingEnabled() const
{
    return getBool("ENABLE_PROGRESS_TRACKING", true);
}

std::string ConfigManager::getLogLevel() const
{
    return getString("LOG_LEVEL", "INFO");
}

// ----------------------------- Helper Functions --------------------------------->

std::string ConfigManager::trim(const std::string& str) const
{
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

bool ConfigManager::parseLine(const std::string& line, std::string& key, std::string& value) const
{
    size_t equalPos = line.find('=');
    if (equalPos == std::string::npos) {
        return false;
    }
    
    key = trim(line.substr(0, equalPos));
    value = trim(line.substr(equalPos + 1));
    
    // Remove quotes if present
    if (value.length() >= 2 && 
        ((value.front() == '"' && value.back() == '"') ||
         (value.front() == '\'' && value.back() == '\''))) {
        value = value.substr(1, value.length() - 2);
    }
    
    return !key.empty();
}

void ConfigManager::setDefaults()
{
    // Server defaults
    config["FRONTEND_PORT"] = "3000";
    config["BACKEND_PORT"] = "8080";
    
    // Storage defaults
    config["STORAGE_DIRECTORY"] = "./uploads/";
    config["STORAGE_MAX_FILE_SIZE"] = "104857600"; // 100MB
    config["STORAGE_CHUNK_SIZE"] = "65536"; // 64KB
    
    // Application defaults
    config["LOG_LEVEL"] = "INFO";
    config["ENABLE_FILE_VERIFICATION"] = "true";
    config["ENABLE_PROGRESS_TRACKING"] = "true";
    
    // Development defaults
    config["DEBUG_MODE"] = "false";
    config["VERBOSE_LOGGING"] = "true";
}

void ConfigManager::logInfo(const std::string& message) const
{
    std::cout << COLOR_BLUE << "[Config] " << message << COLOR_RESET << std::endl;
}

void ConfigManager::logError(const std::string& message) const
{
    std::cerr << COLOR_RED << "[Config] ERROR: " << message << COLOR_RESET << std::endl;
}