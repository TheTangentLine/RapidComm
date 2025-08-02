# HTTP-FTP Server Makefile
# ========================

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
DEBUGFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -g -O0 -DDEBUG
INCLUDES := -Isrc/services
LIBS := -pthread

# macOS specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CXXFLAGS += -D_DARWIN_C_SOURCE
    DEBUGFLAGS += -D_DARWIN_C_SOURCE
endif

# Source and header files
SRCDIR := src/services
SOURCES := $(SRCDIR)/main.cpp \
           	$(SRCDIR)/http/HttpHandler.cpp \
	$(SRCDIR)/socket/Socket.cpp \
	$(SRCDIR)/ftp/FtpHandler.cpp \
	$(SRCDIR)/server/ServerManager.cpp

HEADERS := $(SRCDIR)/http/HttpHandler.hpp \
	$(SRCDIR)/socket/Socket.hpp \
	$(SRCDIR)/ftp/FtpHandler.hpp \
	$(SRCDIR)/server/ServerManager.hpp

# Object files
OBJDIR := build/obj
OBJECTS := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Binary
BINDIR := build/bin
BINARY := $(BINDIR)/http-ftp-server

# Directories that need to be created
DIRS := $(OBJDIR) $(OBJDIR)/http $(OBJDIR)/socket $(OBJDIR)/ftp $(OBJDIR)/server $(BINDIR) uploads

# Default target
.PHONY: all
all: $(BINARY)

# Debug build
.PHONY: debug
debug: CXXFLAGS = $(DEBUGFLAGS)
debug: $(BINARY)

# Create directories
$(DIRS):
	@mkdir -p $@

# Compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(DIRS)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link object files to create binary
$(BINARY): $(OBJECTS) | $(BINDIR)
	@echo "Linking $(BINARY)..."
	@$(CXX) $(OBJECTS) $(LIBS) -o $@
	@echo "Build completed: $(BINARY)"

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build

# Deep clean (removes uploads directory too)
.PHONY: clean-all
clean-all: clean
	@echo "Cleaning uploads directory..."
	@rm -rf uploads

# Run the server
.PHONY: run
run: $(BINARY)
	@cd $(BINDIR) && ./http-ftp-server

# Install binary to system (optional)
.PHONY: install
install: $(BINARY)
	@echo "Installing to /usr/local/bin..."
	@sudo cp $(BINARY) /usr/local/bin/
	@echo "Installation completed!"

# Uninstall binary from system
.PHONY: uninstall
uninstall:
	@echo "Removing from /usr/local/bin..."
	@sudo rm -f /usr/local/bin/http-ftp-server
	@echo "Uninstallation completed!"

# Show help
.PHONY: help
help:
	@echo "HTTP-FTP Server Makefile"
	@echo "========================"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build the server (default, release mode)"
	@echo "  debug     - Build in debug mode with debugging symbols"
	@echo "  run       - Build and run the server"
	@echo "  clean     - Remove build artifacts"
	@echo "  clean-all - Remove build artifacts and uploads"
	@echo "  install   - Install binary to /usr/local/bin"
	@echo "  uninstall - Remove binary from /usr/local/bin"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make           # Build in release mode"
	@echo "  make debug     # Build in debug mode"
	@echo "  make run       # Build and run"
	@echo "  make clean run # Clean build and run"
	@echo ""
	@echo "Build Info:"
	@echo "  Compiler: $(CXX)"
	@echo "  Flags:    $(CXXFLAGS)"
	@echo "  System:   $(UNAME_S)"
	@echo ""

# Show build status
.PHONY: status
status:
	@echo "HTTP-FTP Server Build Status"
	@echo "============================"
	@echo ""
	@if [ -f "$(BINARY)" ]; then \
		echo "✅ Binary exists: $(BINARY)"; \
		echo "📅 Built: $$(stat -c %y $(BINARY) 2>/dev/null || stat -f %Sm $(BINARY) 2>/dev/null)"; \
		echo "📏 Size: $$(du -h $(BINARY) | cut -f1)"; \
	else \
		echo "❌ Binary not found: $(BINARY)"; \
		echo "   Run 'make' to build the project"; \
	fi
	@echo ""
	@if [ -d "uploads" ]; then \
		echo "📁 Uploads directory exists"; \
		echo "📊 Files: $$(find uploads -type f | wc -l)"; \
	else \
		echo "📁 Uploads directory will be created on first run"; \
	fi
	@echo ""

# Force rebuild
.PHONY: rebuild
rebuild: clean all

# Print variables for debugging Makefile
.PHONY: vars
vars:
	@echo "Makefile Variables:"
	@echo "==================="
	@echo "CXX       = $(CXX)"
	@echo "CXXFLAGS  = $(CXXFLAGS)"
	@echo "INCLUDES  = $(INCLUDES)"
	@echo "LIBS      = $(LIBS)"
	@echo "SOURCES   = $(SOURCES)"
	@echo "OBJECTS   = $(OBJECTS)"
	@echo "BINARY    = $(BINARY)"
	@echo "UNAME_S   = $(UNAME_S)"