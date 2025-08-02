#!/bin/bash

# HTTP-FTP Server Quick Start Script
# ==================================

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}"
    echo "=================================================="
    echo "  HTTP-FTP File Upload Server Quick Start"
    echo "=================================================="
    echo -e "${NC}"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_header

# Check if binary exists
if [ ! -f "build/bin/http-ftp-server" ]; then
    print_info "Building the server..."
    make
    echo ""
fi

print_success "Server is ready!"
echo ""

# Show URLs
echo -e "${YELLOW}Server URLs:${NC}"
echo "  Frontend:  http://localhost:3000"
echo "  Backend:   http://localhost:8080" 
echo "  FTP:       localhost:2121"
echo ""

echo -e "${YELLOW}Quality Features:${NC}"
echo "  • Byte-perfect file transfers with integrity verification"
echo "  • Chunked upload with 64KB optimization"
echo "  • Automatic retry logic for network reliability"
echo "  • Real-time progress tracking for large files"
echo ""

echo -e "${YELLOW}How to test:${NC}"
echo "  1. Visit http://localhost:3000 in your browser"
echo "  2. Drag and drop any file or click to browse"
echo "  3. Upload and watch quality optimization in console"
echo "  4. Check uploads/ directory for perfectly preserved files"
echo ""

echo -e "${YELLOW}Control:${NC}"
echo "  • Press Ctrl+C to stop all servers"
echo "  • Files maintain exact quality across machines"
echo ""

echo "=================================================="
echo "Starting servers... (Press Ctrl+C to stop)"
echo "=================================================="
echo ""

# Run the server
exec make run