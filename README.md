# RapidComm v1.0.0

 A modern, high-performance **C++ file sharing server** with an intuitive web interface and lightning-fast upload capabilities.

## 3 Key Strengths

1. **Lightning-Fast Multi-File Uploads** - Upload multiple files simultaneously with real-time progress tracking and automatic retry mechanisms
2. **Zero-Configuration Setup** - Start sharing files instantly with a single command - no complex installation or setup required  
3. **Universal Compatibility** - Works on any device with a web browser, supports 70+ file types, and enables seamless network sharing

## Download & Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/RapidComm.git
cd RapidComm
./RapidCommDesktop.sh
```

2. Open your browser and navigate to:
   - Local access: `http://localhost:3000`
   - Network sharing: `http://YOUR_IP:3000`

## How to Use

### Upload Files

1. **Web Interface**: Open `http://localhost:3000` in any modern browser
2. **Drag & Drop**: Simply drag files from your computer into the upload area
3. **Browse Files**: Click the upload area to select files using the file dialog
4. **Multiple Files**: Select multiple files at once for batch uploading
5. **Progress Tracking**: Watch real-time upload progress for each file

### Network Sharing

1. Start RapidComm on your computer
2. Find your IP address (displayed in terminal or use `ifconfig`/`ipconfig`)
3. Share the URL: `http://YOUR_IP:3000`
4. Others on the same network can access and upload files

### Supported File Types

RapidComm supports virtually any file type including:
- **Code**: .cpp, .py, .js, .html, .css, .java, .go, .rs
- **Documents**: .pdf, .docx, .txt, .md, .xlsx
- **Media**: .mp4, .mp3, .png, .jpg, .gif, .svg
- **Archives**: .zip, .tar, .gz, .rar
- **Applications**: .exe, .app, .dmg, .deb

## Configuration

Customize RapidComm by editing `config.env`:

```bash
# Server ports
FRONTEND_PORT=3000
BACKEND_PORT=8080

# File storage location
STORAGE_DIRECTORY=/path/to/your/upload/folder

# Upload limits (optional)
MAX_FILE_SIZE=100MB
MAX_CONCURRENT_UPLOADS=10
```

## System Requirements

- **Operating System**: Linux, macOS, or Windows
- **Compiler**: C++17 compatible compiler (GCC 7+, Clang 6+, MSVC 2019+)
- **Build Tools**: Make or CMake
- **Browser**: Any modern browser with ES6 module support

## Architecture

```
Web Browser ──→ HTTP Server ──→ File Storage
(Port 3000)     (Port 8080)     (uploads/)
```

The application consists of:
- **Backend (C++)**: High-performance HTTP server with file upload processing (2,111 lines)
- **Frontend (Web)**: Modern HTML5/CSS3/ES6 modular interface
- **Storage**: Configurable file storage with integrity verification

**Primary Language**: C++ (backend server handles all file operations, networking, and storage)

## Features

- **Real-time Progress Tracking**: See upload progress with byte-level precision
- **File Integrity Verification**: Automatic hash verification ensures file integrity
- **Responsive Design**: Works on desktop, tablet, and mobile devices
- **Accessibility**: Full keyboard navigation and screen reader support
- **Error Recovery**: Automatic retry on network failures
- **Drag & Drop**: Intuitive file selection with visual feedback
- **Multi-browser Support**: Compatible with Chrome, Firefox, Safari, and Edge

## Troubleshooting

### Common Issues

**Port Already in Use**
```bash
# Change ports in config.env
FRONTEND_PORT=3001
BACKEND_PORT=8081
```

**Permission Denied**
```bash
# Ensure write permissions for upload directory
chmod 755 uploads/
```

**Firewall Blocking Network Access**
```bash
# Allow ports through firewall (Linux/macOS)
sudo ufw allow 3000
sudo ufw allow 8080
```

## Development

### Building from Source

```bash
# Clone repository
git clone https://github.com/yourusername/RapidComm.git
cd RapidComm

# Build backend
make clean && make

# Run tests (if available)
make test
```

### Project Structure

```
RapidComm/
├── src/
│   ├── interface/          # Web frontend
│   │   ├── index.html      # Main upload page
│   │   ├── styles/         # CSS stylesheets
│   │   └── js/             # JavaScript modules
│   └── services/           # C++ backend
├── uploads/                # File storage directory
└── config.env            # Configuration file
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests
5. Submit a pull request

## License

MIT License - see LICENSE file for details

## Version History

- **v1.0.0** - Initial release with modern web interface and modular architecture
- Complete refactoring of frontend with ES6 modules
- Enhanced accessibility and performance optimizations
- Comprehensive error handling and retry mechanisms
