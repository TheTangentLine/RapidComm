# RapidComm - File Upload Server

Modern HTTP file upload server built with C++.

## Architecture

```
┌─────────────┐    HTTP POST     ┌─────────────┐    Direct I/O    ┌─────────────┐
│   Frontend  │ ───────────────→ │   Backend   │ ───────────────→ │   Storage   │
│             │                  │             │                  │   Service   │
│ • File Hash │ ← Upload Progress│ • Integrity │ ← Hash Verify ── │ • Atomic    │
│ • Progress  │   & Verification │ • Multipart │   & Chunked I/O  │ • Verified  │
│ • Retry     │                  │ • CORS      │                  │ • Safe Path │
└─────────────┘                  └─────────────┘                  └─────────────┘
      :3000                            :8080                         ./uploads/
```

**Workflow**: Client calculates hash → Upload with integrity data → Backend verifies → Storage writes atomically → Hash comparison → Success response

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
