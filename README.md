# Evosim

A simple evolution simulator where creatures (peeps) evolve neural networks to survive in different environments. Watch natural selection in action as peeps learn to navigate to survival zones, avoid obstacles, find food, and more.

## Demo

https://youtu.be/z5KyCMiMJRY

## Features

- **Neural Network Brains**: Each peep has a simple neural network that controls its behavior
- **Genetic Evolution**: Peeps that survive pass their genes to the next generation with mutations
- **Multiple Survival Modes**: Different zone configurations to evolve different behaviors
- **Configurable Parameters**: Adjust population size, mutation rates, world size, and more
- **Built-in Presets**: Migration, Foraging, Maze Runner, Hunger Games, and more
- **Real-time Statistics**: Track survival rates and population fitness across generations

## Requirements

- C++17 compiler
- CMake 3.16+
- SFML 3.0+

## Installation

### macOS (Homebrew)

```bash
brew install sfml
```

### Linux (Ubuntu/Debian)

Install SFML 3 dependencies:

```bash
sudo apt update
sudo apt install \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libfreetype-dev \
    libflac-dev \
    libvorbis-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev
```

Note: You may need to build SFML 3 from source if your distribution only has SFML 2.x in repositories.

### Windows

Download SFML 3 from https://www.sfml-dev.org/download.php and set up CMake to find it.

## Building

```bash
cmake -B build
cmake --build build
```

## Running

```bash
./build/bin/evosim
```

## Controls

- **Space**: Pause/Resume simulation
- **R**: Reset simulation
- **Mouse**: Interact with UI sliders and buttons
- Use the control panel on the right to adjust simulation parameters

## Project Structure

```
src/
├── core/           # Configuration and statistics
├── entities/       # Peeps, food, obstacles, zones
├── neural/         # Brain and gene encoding
├── render/         # SFML rendering and input handling
├── simulation/     # Main simulation logic
├── ui/             # UI widgets (sliders, buttons)
└── main.cpp
```

## License

See [LICENSE.md](LICENSE.md)
