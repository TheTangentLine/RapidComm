# HTTP-FTP File Upload Server

A fast file upload server that helps you transfer files quickly within the same network while maintaining perfect quality. Built in C++17 with HTTP frontend, REST API backend, and FTP storage service featuring advanced integrity verification and optimization.

## Quick Start

To run the server, simply execute:

```bash
./quick-start.sh
```

This will automatically build and start all services. Once running, visit `http://localhost:3000` to upload files.

## What it does

This application provides a fast way to upload and share files within your local network with guaranteed quality preservation. Features include:

- **Quality Optimization**: Byte-perfect file transfers with integrity verification
- **Checksum Verification**: Multi-layer hash validation ensures no data corruption
- **Chunk-based Transfer**: Optimized 64KB chunks with retry logic for reliability
- **Progress Tracking**: Real-time transfer progress for large files
- **Error Recovery**: Automatic retry mechanisms for network issues

The system consists of three services:

- **Frontend Server** (Port 3000): Web interface for file uploads
- **Backend Server** (Port 8080): REST API to handle file processing
- **FTP Server** (Port 2121): File storage and retrieval service

## How to use

1. Run `./quick-start.sh`
2. Open your browser to `http://localhost:3000`
3. Drag and drop files or click to browse
4. Files are automatically processed and stored
5. Check the `uploads/` directory for your files

## Building manually

If you prefer to build manually:

```bash
make clean
make
make run
```

## Requirements

- C++17 compatible compiler (GCC 8+ or Clang 7+)
- POSIX threads support
- Make build system

## File Storage

All uploaded files are stored in the `uploads/` directory. The server automatically creates this directory if it doesn't exist.

## Stopping the server

Press `Ctrl+C` to stop all servers gracefully.

## Architecture

The server uses a three-tier architecture with separate processes for frontend serving, API handling, and file storage. This provides better performance and reliability for file transfers within your network.
