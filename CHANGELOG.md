# Changelog

All notable changes to the HTTP-FTP File Upload Server project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Initial project setup and architecture
- Three-tier server architecture (Frontend/Backend/FTP)
- Real-time file upload with progress tracking
- Content-Length based streaming upload support
- Comprehensive multipart form data parsing
- Automatic file type detection and content analysis
- Secure filename sanitization
- Cross-platform build system (CMake + Make)
- Automated build scripts with multiple options
- Professional documentation and setup guides

### Enhanced

- Robust multipart boundary parsing with multiple pattern support
- Improved error handling throughout the upload pipeline
- Comprehensive debug logging for troubleshooting
- Dynamic URL configuration (no hardcoded localhost)
- Memory-efficient streaming for large file uploads

### Security

- Path traversal protection in file storage
- Input validation for all user data
- Safe filename handling with character sanitization
- CORS support for cross-origin requests

## [1.0.0] - 2024-XX-XX

### Added

- Initial release of HTTP-FTP File Upload Server
- Complete three-tier microservices architecture
- Frontend HTTP server on port 3000
- Backend REST API server on port 8080
- FTP storage server on port 2121
- Responsive web interface with drag-and-drop upload
- Real-time progress tracking during uploads
- File content analysis and preview
- Multi-threaded concurrent connection handling
- Cross-platform compatibility (macOS/Linux)
- Zero external dependencies (pure C++17)
- Comprehensive build system with multiple options
- Professional documentation and setup guides
- MIT license for open source distribution

### Technical Features

- Custom HTTP/FTP protocol implementation
- BSD sockets for network communication
- POSIX threads for concurrency
- std::filesystem for file operations
- Streaming upload support for large files
- Memory-efficient chunk-based processing
- Graceful shutdown handling with signal management

### Developer Experience

- CMake and Make build system support
- Automated build scripts with command-line options
- Debug and release build configurations
- Comprehensive error reporting and logging
- Code formatting and style guidelines
- Contributing guidelines and development workflow

---

## Release Notes Format

### Categories

- **Added** for new features
- **Changed** for changes in existing functionality
- **Deprecated** for soon-to-be removed features
- **Removed** for now removed features
- **Fixed** for any bug fixes
- **Security** for vulnerability fixes
- **Enhanced** for improvements to existing features

### Version Numbering

- **Major** (X.0.0): Breaking changes or significant new features
- **Minor** (0.X.0): New features that are backward compatible
- **Patch** (0.0.X): Bug fixes and small improvements

### Links

- [Keep a Changelog](https://keepachangelog.com/)
- [Semantic Versioning](https://semver.org/)
- [Project Repository](https://github.com/yourusername/http-ftp-server)
