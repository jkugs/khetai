# Makefile for Khet FLTK Application

# Compiler settings - Can be customized.
CXX = g++
CXXFLAGS = -std=c++11 $(shell fltk-config --cxxflags)
LDFLAGS = $(shell fltk-config --ldflags)

# Application name
APP = khet

# Source files
SRC = khet.cpp

# Object files
OBJ = $(SRC:.cpp=.o)

# Default target
all: $(APP)

# Link the application
$(APP): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

# Compile source files into object files
%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS)

# Clean target
clean:
	rm -f $(OBJ) $(APP)

.PHONY: all clean