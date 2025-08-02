#!/bin/bash

# HTTP-FTP Server Quick Start Script
# ==================================

set -e

# Colors for output (check if terminal supports colors)
if [[ -t 1 ]] && command -v tput >/dev/null 2>&1 && [[ $(tput colors) -ge 8 ]]; then
    GREEN='\033[0;32m'
    BLUE='\033[0;34m'
    YELLOW='\033[1;33m'
    RED='\033[0;31m'
    BOLD='\033[1m'
    NC='\033[0m' # No Color
else
    GREEN=''
    BLUE=''
    YELLOW=''
    RED=''
    BOLD=''
    NC=''
fi

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

print_success "Build completed! Starting servers..."
echo ""

# Run the server
exec make run