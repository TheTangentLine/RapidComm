# RapidComm File Upload Server

A fast C++ file upload server with real-time web interface, optimized for speed and data integrity.

## Features

- Ultra-fast uploads with instant initiation
- Atomic file operations for data integrity
- Real-time progress tracking
- External configuration via config.env
- Video file optimization

## Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Browser   │───▶│   HTTP Server   │───▶│ Storage Service │
│  (Frontend UI)  │    │ (Backend API)   │    │ (File Storage)  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
       Port 3000              Port 8080       Configurable Path
```

## Quick Start

```bash
./quick-start.sh
```

Then open: http://localhost:3000

## Configuration

Edit `config.env`:
```bash
FRONTEND_PORT=3000
BACKEND_PORT=8080
STORAGE_DIRECTORY=/path/to/uploads
STORAGE_MAX_FILE_SIZE=2147483648
```

## Requirements

- C++17 compiler
- Make build system
- Linux, macOS, or Windows (WSL recommended)