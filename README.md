# RapidComm - File Upload Server

Modern HTTP file upload server built with C++.

## Architecture

```
Frontend (3000) → Backend (8080) → Storage Service → ./uploads/
```

User uploads file via web interface, backend processes it, storage service saves to disk.

## File Structure

```
src/
├── services/
│   ├── main.cpp
│   ├── config/
│   │   ├── ConfigManager.hpp
│   │   └── ConfigManager.cpp
│   ├── http/
│   │   ├── HttpHandler.hpp
│   │   └── HttpHandler.cpp
│   ├── server/
│   │   ├── ServerManager.hpp
│   │   └── ServerManager.cpp
│   ├── socket/
│   │   ├── Socket.hpp
│   │   └── Socket.cpp
│   └── storage/
│       ├── StorageService.hpp
│       └── StorageService.cpp
└── interface/
    ├── index.html
    └── nothingToExplore.html
```

## How to Run

```bash
# Quick start (recommended)
./quick-start.sh

# Or manually
make build
make run

# Access
http://localhost:3000
```

## OS Support

- ✅ Linux
- ✅ macOS
- ✅ Windows (with MinGW/MSVC)

## Requirements

- C++17 compiler
- Make
- CMake 3.12+
