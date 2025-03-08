cmake_minimum_required(VERSION 3.10)
project(KhetSDL C)

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

# Emscripten options for wasm target
if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    add_compile_options(-sUSE_SDL=3)
    add_link_options(
        -sUSE_SDL=3
        -sALLOW_MEMORY_GROWTH=1
        -sNO_EXIT_RUNTIME=1
    )
endif()

# Link against SDL3 and math library
target_link_libraries(khet-sdl SDL3::SDL3 m)