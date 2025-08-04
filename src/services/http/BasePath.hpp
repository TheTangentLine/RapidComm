#ifndef BASE_PATH_HPP
#define BASE_PATH_HPP

#include <string>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <limits.h> 
#else // Assuming Linux
#include <unistd.h> 
#endif

// This function determines the absolute path to the directory containing the executable.
// This is crucial for finding resource files (like config.env, HTML files)
// reliably, especially when the app is bundled.
static std::string getBasePath() {
    std::filesystem::path execPath;

#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    execPath = std::filesystem::path(buffer);
#elif __APPLE__
    char rawPathName[PATH_MAX];
    char realPathName[PATH_MAX];
    uint32_t rawPathSize = (uint32_t)sizeof(rawPathName);

    if (_NSGetExecutablePath(rawPathName, &rawPathSize) == 0) {
        // realpath() resolves all symlinks, and corrects .. and . from path
        if (realpath(rawPathName, realPathName) != NULL) {
             execPath = std::filesystem::path(realPathName);
        } else {
            // This is a fallback in case realpath fails for some reason
            execPath = std::filesystem::path(rawPathName);
        }
    } else {
        // This should not happen, but as a fallback.
        return "./"; 
    }
#else // Assuming Linux
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        execPath = std::filesystem::path(buffer);
    } else {
        return "./";
    }
#endif

    // The result of parent_path() on a file path is the directory containing it.
    if (execPath.has_parent_path()) {
        return execPath.parent_path().string();
    }

    return "./"; // Final fallback
}

#endif // BASE_PATH_HPP
