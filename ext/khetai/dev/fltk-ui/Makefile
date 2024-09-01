# Makefile for Khet FLTK Application

# Compiler settings - Can be customized.
CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 $(shell fltk-config --cxxflags) -fPIC
CFLAGS = -std=c11 -fPIC
LDFLAGS = $(shell fltk-config --ldflags) -lfltk_images -lfltk_png -lfltk_z -ldl

# Application name
APP = khet

# Source directory
SRC_DIR = .

# Build directory
BUILD_DIR = build

# Source files
SRC_CPP = $(wildcard $(SRC_DIR)/*.cpp)
SRC_C = ../../khetai_lib.c

# Object files
OBJ_CPP = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_CPP))
OBJ_C = $(patsubst ../../%.c,$(BUILD_DIR)/%.o,$(SRC_C))

# All object files
OBJ = $(OBJ_CPP) $(OBJ_C)

# Header files
DEPS = $(wildcard $(SRC_DIR)/*.h) ../../khetai_lib.h

# AI Library
AI_LIB = libkhetai.so

# Default target
all: $(BUILD_DIR) $(APP) $(AI_LIB)

# Create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link the application
$(APP): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile C++ source files into object files
# Include header files as dependencies
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEPS)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Compile C source files into object files
$(BUILD_DIR)/%.o: ../../%.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Compile AI library
$(AI_LIB): $(OBJ_C)
	$(CC) -shared -o $@ $^ $(CFLAGS)

# Clean target
clean:
	rm -f $(OBJ) $(APP) $(AI_LIB)
	rm -rf $(BUILD_DIR)

.PHONY: all clean
