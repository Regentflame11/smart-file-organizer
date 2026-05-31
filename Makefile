CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic
TARGET   = organizer
PREFIX   = /usr/local/bin

# Detect OS
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    LDFLAGS = -lpthread
endif

ifeq ($(UNAME_S),Darwin)
    LDFLAGS = -lpthread
endif

# Windows (MinGW)
ifeq ($(OS),Windows_NT)
    TARGET  = organizer.exe
    LDFLAGS = -mwindows
endif

.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): organizer.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) organizer.cpp $(LDFLAGS)
	@echo ""
	@echo "  Build successful: ./$(TARGET)"
	@echo "  Run './$(TARGET) --help' for usage info."
	@echo ""

clean:
	rm -f $(TARGET)

install: $(TARGET)
	install -Dm755 $(TARGET) $(PREFIX)/$(TARGET)
	@echo "Installed to $(PREFIX)/$(TARGET)"

uninstall:
	rm -f $(PREFIX)/$(TARGET)
	@echo "Removed $(PREFIX)/$(TARGET)"
