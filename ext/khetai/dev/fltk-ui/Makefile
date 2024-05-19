# Makefile for Khet FLTK Application

# Compiler settings - Can be customized.
CXX = g++
CXXFLAGS = -std=c++17 $(shell fltk-config --cxxflags)
LDFLAGS = $(shell fltk-config --ldflags) -lfltk_images -lfltk_png -lfltk_z

# Application name
APP = khet

# Source directory
SRC_DIR = .

# Source files
SRC = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJ = $(SRC:.cpp=.o)

# Header files
DEPS = $(wildcard $(SRC_DIR)/*.h)

# Default target
all: $(APP)

# Link the application
$(APP): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile source files into object files
# Include header files as dependencies
%.o: %.cpp $(DEPS)
	$(CXX) -c $< $(CXXFLAGS)

# Clean target
clean:
	rm -f $(OBJ) $(APP)

.PHONY: all clean
