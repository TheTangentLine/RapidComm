#!/bin/bash

# HTTP-FTP Server Build Script
# ============================

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check required tools
print_status "Checking required tools..."

if ! command_exists cmake; then
    print_error "CMake is not installed. Please install CMake first."
    print_status "On macOS: brew install cmake"
    print_status "On Ubuntu: sudo apt-get install cmake"
    exit 1
fi

if ! command_exists make; then
    print_error "Make is not installed. Please install build tools."
    print_status "On macOS: xcode-select --install"
    print_status "On Ubuntu: sudo apt-get install build-essential"
    exit 1
fi

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_AFTER_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -r|--run)
            RUN_AFTER_BUILD=true
            shift
            ;;
        -h|--help)
            echo "HTTP-FTP Server Build Script"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  -d, --debug     Build in Debug mode (default: Release)"
            echo "  -c, --clean     Clean build directory before building"
            echo "  -r, --run       Run the server after successful build"
            echo "  -h, --help      Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                  # Build in Release mode"
            echo "  $0 --debug --run    # Build in Debug mode and run"
            echo "  $0 --clean --run    # Clean build and run"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Print build configuration
echo ""
print_status "HTTP-FTP Server Build Configuration"
echo "=================================="
echo "Build Type: $BUILD_TYPE"
echo "Clean Build: $CLEAN_BUILD"
echo "Run After Build: $RUN_AFTER_BUILD"
echo "=================================="
echo ""

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_status "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
print_status "Setting up build directory..."
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring project with CMake..."
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE .. || {
    print_error "CMake configuration failed!"
    exit 1
}

# Build the project
print_status "Building project..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || {
    print_error "Build failed!"
    exit 1
}

print_success "Build completed successfully!"

# Show binary location
BINARY_PATH="$(pwd)/bin/http-ftp-server"
print_status "Binary location: $BINARY_PATH"

# Create uploads directory if it doesn't exist
print_status "Setting up uploads directory..."
mkdir -p uploads

# Run the server if requested
if [ "$RUN_AFTER_BUILD" = true ]; then
    echo ""
    print_status "Starting HTTP-FTP Server..."
    echo "=================================="
    echo "Frontend: http://localhost:3000"
    echo "Backend:  http://localhost:8080"
    echo "FTP:      localhost:2121"
    echo "=================================="
    echo "Working directory: $(pwd)/bin"
    echo "HTML files: ../../src/interface/"
    echo "Press Ctrl+C to stop the server"
    echo ""
    
    # Change to the directory with the binary to ensure proper relative paths
    cd bin
    exec ./http-ftp-server
else
    echo ""
    print_success "Build completed! To run the server:"
    echo ""
    echo "  cd build/bin"
    echo "  ./http-ftp-server"
    echo ""
    echo "Or use: make run"
    echo ""
fi