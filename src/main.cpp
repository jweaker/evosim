// main.cpp - Entry point for the Evolution Simulator

#include "render.hpp"
#include "simulation.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
  // Window configuration - higher resolution for sharper rendering
  const int windowWidth = 1600;
  const int windowHeight = 1000;
  const int frameLimit = 60;

  // Create window
  sf::RenderWindow window(
      sf::VideoMode({static_cast<unsigned int>(windowWidth),
                     static_cast<unsigned int>(windowHeight)}),
      "Evolution Simulator", sf::Style::Default);
  window.setFramerateLimit(frameLimit);

  // Create renderer
  Renderer renderer;
  renderer.initialize(windowWidth, windowHeight);

  // Try to load font from multiple locations
  // (works whether running from project root or build directory)
  bool fontLoaded = false;

  if (!fontLoaded) {
    fontLoaded = renderer.loadFont("src/assets/arial.ttf");
  }
  if (!fontLoaded) {
    fontLoaded = renderer.loadFont("../src/assets/arial.ttf");
  }
  if (!fontLoaded) {
    fontLoaded = renderer.loadFont("assets/arial.ttf");
  }
  if (!fontLoaded) {
    fontLoaded = renderer.loadFont("arial.ttf");
  }

  if (!fontLoaded) {
    std::cerr << "Warning: Could not load font from any location.\n";
    std::cerr << "Tried: src/assets/arial.ttf, ../src/assets/arial.ttf, "
                 "assets/arial.ttf, arial.ttf\n";
    std::cerr << "UI text will not be displayed.\n";
  }

  // Create and initialize simulation with Migration preset as default
  Simulation sim;
  sim.applyPreset(0); // Migration - a good starting scenario

  // Run the main loop
  runMainLoop(window, sim, renderer);

  return 0;
}
