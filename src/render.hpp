
#pragma once
#include "input.hpp"
#include "simulation.hpp"
#include <SFML/Graphics.hpp>

namespace Window {
constexpr int Width = 1000, Height = 800;
}
namespace Frame {
constexpr int Limit = 60;
}

void runLoop(sf::RenderWindow &window, Simulation &sim,
             class Renderer &renderer, InputHandler &input);

class Renderer {
public:
  Renderer(sf::RenderWindow &win, const sf::Font &font);
  void draw(const Simulation &sim, bool speedUp, SurvivalMode mode,
            bool paused);

private:
  void drawLegend(SurvivalMode mode);
  const sf::Font &font;
  sf::RenderWindow &window;
  sf::Text statsText;
};
