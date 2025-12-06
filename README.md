# Evosim

An evolution simulator where creatures (peeps) evolve neural networks to survive in different environments. Watch natural selection in action as peeps learn to navigate to survival zones, avoid obstacles, find food, and more.

## Features

- **Neural Network Brains**: Each peep has a neural network with configurable hidden neurons and genes
- **Genetic Evolution**: Survivors pass genes to the next generation with mutations and crossover
- **Hunger System**: Optional hunger mechanics where peeps must find food to survive
- **Obstacles**: Draw obstacles that peeps must navigate around
- **Survival Zones**: Define zones where peeps must reach to survive each generation
- **Spawn Zones**: Control where peeps spawn at the start of each generation
- **Real-time Statistics**: Track survival rates across generations with live graphs
- **DNA Visualization**: Click on any peep to view its neural network and genome
- **Speed Control**: Run the simulation at up to 100x speed

## Presets

- **Migration**: Peeps must travel from left spawn zone to right survival zone
- **Foraging**: Hunger enabled, peeps must find food to survive
- **Maze Runner**: Navigate through obstacles to reach the survival zone
- **Hunger Games**: Scarce food with obstacles, survival of the fittest
- **Big Brains**: Large neural networks with complex behaviors
- **Speed Demons**: Fast generations with quick evolution

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

| Key | Action |
|-----|--------|
| Space | Pause/Resume |
| R | Reset simulation |
| E | Toggle obstacle editing mode |
| Z | Toggle survival zone editing mode |
| S | Toggle spawn zone editing mode |
| Escape | Exit editing mode / Deselect peep |
| Up/Down | Increase/Decrease simulation speed |
| 1, 2, 5, 0 | Set speed to 1x, 2x, 5x, 10x |

### Editing Mode

When in editing mode (obstacles, survival zones, or spawn zones):
- **Left click + drag**: Draw a new rectangle
- **Right click**: Delete existing rectangle

### Mouse

- **Click on peep**: Select and view its DNA/neural network
- **Click on sliders**: Adjust simulation parameters

## Project Structure

```
src/
├── core/           # Configuration and statistics
├── entities/       # Peeps, food, obstacles, zones
├── neural/         # Brain and gene encoding
├── render/         # SFML rendering and input handling
├── simulation/     # Main simulation logic
├── ui/             # UI widgets (sliders, buttons, toggles)
└── main.cpp
```

## License

See [LICENSE.md](LICENSE.md)
