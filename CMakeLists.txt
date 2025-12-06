cmake_minimum_required(VERSION 3.16)
project(Evosim LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find system SFML (installed via Homebrew)
find_package(SFML 3 COMPONENTS Graphics REQUIRED)

# Create executable with all source files
add_executable(evosim
    src/main.cpp
    
    # Simulation (split into 4 files)
    src/simulation/simulation.cpp
    src/simulation/peep_logic.cpp
    src/simulation/generation.cpp
    src/simulation/world.cpp
    
    # Renderer (split into 4 files)
    src/render/renderer.cpp
    src/render/draw_world.cpp
    src/render/draw_ui.cpp
    src/render/input.cpp
    
    # Core
    src/core/config.cpp
    src/core/stats.cpp
    
    # Entities
    src/entities/rect.cpp
    src/entities/spawn_zone.cpp
    src/entities/food.cpp
    src/entities/peep.cpp
    
    # Neural
    src/neural/gene.cpp
    src/neural/brain.cpp
    
    # UI
    src/ui/widgets.cpp
)

# Set C++17 standard
target_compile_features(evosim PRIVATE cxx_std_17)

# Link SFML
target_link_libraries(evosim PRIVATE SFML::Graphics)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/src/assets DESTINATION ${CMAKE_BINARY_DIR}/bin)
