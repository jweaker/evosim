#include "input.hpp"
#include "render.hpp"
#include "simulation.hpp"
#include <SFML/Graphics.hpp>

int main() {
  auto window = sf::RenderWindow(sf::VideoMode({1000, 800}), "Evosim");
  window.setFramerateLimit(Frame::Limit);

  sf::Font font("src/assets/arial.ttf");

  Simulation sim;
  sim.initPeeps();

  Renderer renderer(window, font);
  InputHandler input;

  runLoop(window, sim, renderer, input);
  return EXIT_SUCCESS;
}
