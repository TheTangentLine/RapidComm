#!/bin/bash

# Fast RapidComm Development Startup
# ==================================

PROJECT_ROOT="/PATH/TO/YOUR/PROJECT_ROOT"
DESKTOP_DIR="$PROJECT_ROOT/rapid-comm-desktop"

echo "⚡ Fast-starting RapidComm Development Environment..."

# Function to open new Terminal tab/window on macOS
open_terminal_tab() {
    local command="$1"
    local title="$2"
    
    osascript <<EOF
tell application "Terminal"
    activate
    delay 0.3
    if (count of windows) = 0 then
        do script "$command"
    else
        tell application "System Events" to tell process "Terminal" to keystroke "t" using command down
        delay 0.8
        do script "$command" in front window
    end if
    delay 0.5
    set custom title of front window to "$title"
    activate
    tell application "System Events"
        tell process "Terminal"
            set frontmost to true
        end tell
    end tell
end tell
EOF
}

echo "🔍 Checking if backend is already built..."
if [ ! -f "$PROJECT_ROOT/build/bin/rapidcomm-server" ]; then
    echo "🔨 Building backend (first time only)..."
    cd "$PROJECT_ROOT"
    make > /dev/null 2>&1
    echo "Backend built successfully!"
fi

# Ensure Tauri-compatible binary exists
echo "🔧 Ensuring Tauri compatibility..."
cp "$PROJECT_ROOT/build/bin/rapidcomm-server" "$PROJECT_ROOT/build/bin/rapidcomm-server-aarch64-apple-darwin" 2>/dev/null || true

# Start backend server and admin interface
echo "🚀 Starting backend server and admin interface..."

# Desktop app: build once, then launch instantly
APP_BUNDLE="$DESKTOP_DIR/src-tauri/target/release/bundle/macos/rapid-comm-desktop.app"

if [ ! -d "$APP_BUNDLE" ]; then
    echo "🔨 Building desktop app (first time only)... (this may take a while)"
    cd "$DESKTOP_DIR"
    npm run tauri build -- --ci > /dev/null 2>&1
    echo "Desktop app built!"
else
    echo "Desktop app already built. Skipping build step."
fi

# Launch the compiled desktop app
if [ -d "$APP_BUNDLE" ]; then
    echo "Launching desktop app..."
    open -a "$APP_BUNDLE"
else
    echo "Desktop app bundle not found even after build. Falling back to slow dev mode..."
    cd "$DESKTOP_DIR" && npm run tauri dev > /dev/null 2>&1 &
fi

# Start backend server in terminal (optimized command)
echo "📡 Opening backend server terminal..."

# Try the AppleScript method first, fallback to simple approach
if ! open_terminal_tab "cd '$PROJECT_ROOT' && ./build/bin/rapidcomm-server && echo 'Server stopped.'" "RapidComm Backend"; then
    echo "AppleScript failed, using alternative method..."
    # Alternative: Open Terminal directly with enhanced activation
    osascript -e "tell application \"Terminal\"
        activate
        delay 0.5
        do script \"cd '$PROJECT_ROOT' && echo '⚡ Starting pre-built server...' && ./build/bin/rapidcomm-server\"
        activate
        tell application \"System Events\"
            tell process \"Terminal\"
                set frontmost to true
            end tell
        end tell
        end tell"
    fi

# ----------------------------------------
# Final output and hints
# ----------------------------------------
echo "Backend terminal opened!"

echo "📋 Quick Access:"
echo "   • Admin Interface: file://$PROJECT_ROOT/src/interface/index.html"
echo "   • API Server: http://localhost:8080"
echo ""
echo "💡 Tip: Open the Admin Interface in your browser for quick uploads."
