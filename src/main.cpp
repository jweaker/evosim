// main.cpp - Entry point for the Evolution Simulator

#include "simulation.hpp"
#include "render.hpp"
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
        "Evolution Simulator",
        sf::Style::Default
    );
    window.setFramerateLimit(frameLimit);
    
    // Create renderer
    Renderer renderer;
    renderer.initialize(windowWidth, windowHeight);
    
    // Try to load font
    if (!renderer.loadFont("src/assets/arial.ttf")) {
        std::cerr << "Warning: Could not load font from src/assets/arial.ttf\n";
        std::cerr << "UI text will not be displayed.\n";
    }
    
    // Create and initialize simulation
    Simulation sim;
    sim.initialize();
    
    // Run the main loop
    runMainLoop(window, sim, renderer);
    
    return 0;
}
