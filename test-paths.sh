#!/bin/bash

# Test HTML Path Resolution
# =========================

echo "Testing HTML file path resolution..."
echo ""

# Check if binary exists
if [ ! -f "build/bin/http-ftp-server" ]; then
    echo "❌ Binary not found. Run 'make' first."
    exit 1
fi

echo "✅ Binary exists: build/bin/http-ftp-server"

# Check HTML files from binary's perspective
cd build/bin

echo "📁 Current working directory: $(pwd)"
echo ""

echo "🔍 Testing HTML file paths..."

if [ -f "../../src/interface/index.html" ]; then
    echo "✅ index.html found at: ../../src/interface/index.html"
    echo "   Size: $(wc -c < ../../src/interface/index.html) bytes"
else
    echo "❌ index.html NOT found at: ../../src/interface/index.html"
fi

if [ -f "../../src/interface/nothingToExplore.html" ]; then
    echo "✅ nothingToExplore.html found at: ../../src/interface/nothingToExplore.html" 
    echo "   Size: $(wc -c < ../../src/interface/nothingToExplore.html) bytes"
else
    echo "❌ nothingToExplore.html NOT found at: ../../src/interface/nothingToExplore.html"
fi

echo ""
echo "📋 HTML file content preview (first 3 lines of index.html):"
echo "-----------------------------------------------------------"
head -3 ../../src/interface/index.html
echo "-----------------------------------------------------------"

echo ""
echo "🎯 Path resolution test completed!"
echo ""
echo "To test the full server:"
echo "  cd ../.."
echo "  make run"
echo "  Open http://localhost:3000 in browser"