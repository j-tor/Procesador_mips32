cmake_minimum_required(VERSION 3.22)
project(VGATextWindow VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# C++20 required
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Dependencies
# ============================================================================
find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)

# ============================================================================
# vgatext — static library (VGA text window + font + framebuffer + keypad)
# ============================================================================
add_library(vgatext STATIC
    vga/VGATextWindow.h
    vga/VGATextWindow.cpp
    vga/VGAFont.h
    vga/VGAFont.cpp
    vga/VGAConstants.h
    vga/VGAColor.h
    vga/VGAFramebuffer.h
    vga/Keypad.h
)

target_include_directories(vgatext PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(vgatext PUBLIC
    ${SDL2_LIBRARIES}
)

# ============================================================================
# vga_test — demo / test executable
# ============================================================================
add_executable(vga_test main.cpp
    vga/VGAFramebuffer.cpp)

target_link_libraries(vga_test PRIVATE
    vgatext
    ${SDL2_LIBRARIES}
    Threads::Threads
)

target_include_directories(vga_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

# ============================================================================
# Compiler warnings
# ============================================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(vgatext PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
    target_compile_options(vga_test PRIVATE -Wall -Wextra -Wpedantic -Wshadow)
endif()

# ============================================================================
# Optional: Address Sanitizer (-DENABLE_ASAN=ON)
# ============================================================================
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(vgatext PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(vgatext PUBLIC -fsanitize=address)
    target_compile_options(vga_test PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_libraries(vga_test PRIVATE -fsanitize=address)
endif()
