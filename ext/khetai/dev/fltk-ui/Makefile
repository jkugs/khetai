# Makefile for both the C++ FLTK GUI and the KhetAI lib as a shared object

# Compiler settings
CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 $(shell fltk-config --cxxflags) -fPIC -MMD -MP
CFLAGS = -std=c11 -fPIC -MMD -MP
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
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Compile C source files into object files
$(BUILD_DIR)/%.o: ../../%.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Compile AI library
$(AI_LIB): $(OBJ_C)
	$(CC) -shared -o $@ $^ $(CFLAGS)

# Include generated dependency files
-include $(OBJ_CPP:.o=.d)
-include $(OBJ_C:.o=.d)

# Clean target
clean:
	rm -f $(OBJ) $(APP) $(AI_LIB) $(BUILD_DIR)/*.d
	rm -rf $(BUILD_DIR)

.PHONY: all clean
