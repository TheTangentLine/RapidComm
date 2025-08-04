# RapidComm - Fast File Sharing Application

A high-performance C++ file sharing server with both web and desktop interfaces, optimized for speed and multiple file uploads.

## Features

- Multiple file uploads with real-time progress tracking
- Support for 70+ file types including programming languages and binary files
- Web interface (browser access) and desktop application (macOS)
- Network sharing across devices on the same network
- File integrity verification and atomic operations
- External configuration via config.env

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Browser   │───▶│   HTTP Server   │───▶│ Storage Service │
│  (Frontend UI)  │    │ (Backend API)   │    │ (File Storage)  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
        Port 3000             Port 8080        Project/uploads/

┌─────────────────┐    ┌─────────────────┐
│  Desktop App    │───▶│  Control Server │
│ (Admin Panel)   │    │   (Port 8081)   │
└─────────────────┘    └─────────────────┘
```

## Quick Start

Then open: http://localhost:3000 or http://YOUR_IP:3000

### Desktop Application (macOS)

```bash
./build.sh
./start-tauri-dev.sh
```

## Usage

**Web Interface**: Select single or multiple files via click/drag-and-drop. Each file uploads individually with progress tracking.

**Desktop App**: Control server start/stop, view network IP for sharing, and monitor server status.

**Supported Files**: Code (.cpp, .py, .js), media (.mp4, .png, .mp3), documents (.pdf, .txt), archives (.zip), binaries (.exe, .app), and more.

## Configuration

Edit `config.env`:

```bash
FRONTEND_PORT=3000
BACKEND_PORT=8080
STORAGE_DIRECTORY=/TO/YOUR/FOLDER
```

## Requirements

- C++17 compiler and Make
- Node.js and Rust (for desktop app)
- Linux, macOS, or Windows

## Network Sharing

1. Start the server
2. Share your IP address (shown in desktop app): `http://YOUR_IP:3000`
3. Same network required

## License

MIT License
