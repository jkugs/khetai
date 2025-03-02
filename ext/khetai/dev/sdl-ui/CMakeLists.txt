cmake_minimum_required(VERSION 3.10)
project(KhetSDL C)  # Specify the language as C

# Find SDL3 package
find_package(SDL3 REQUIRED)

# Define the source files
set(SOURCES
    khet-sdl.c
    drawing.c
)

# Add the executable
add_executable(khet-sdl ${SOURCES})

# Include current directory for headers
target_include_directories(khet-sdl PRIVATE ${CMAKE_SOURCE_DIR})

# Link against SDL3 and math library
target_link_libraries(khet-sdl SDL3::SDL3 m)