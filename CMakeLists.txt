cmake_minimum_required(VERSION 3.16)
project(Evosim LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find system SFML (installed via Homebrew)
find_package(SFML 3 COMPONENTS Graphics REQUIRED)

# Create executable with all source files
add_executable(evosim
    src/main.cpp
    src/simulation.cpp
    src/render.cpp
    src/types.cpp
)

# Set C++17 standard
target_compile_features(evosim PRIVATE cxx_std_17)

# Link SFML
target_link_libraries(evosim PRIVATE SFML::Graphics)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/src/assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
