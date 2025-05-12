
#include "input.hpp"
#include <SFML/Window/Keyboard.hpp>

void InputHandler::processEvent(const sf::Event &e) {
  if (e.is<sf::Event::KeyPressed>()) {

    switch (e.getIf<sf::Event::KeyPressed>()->code) {
    case sf::Keyboard::Key::Space:
      speedUp = !speedUp;
      break;
    case sf::Keyboard::Key::Num1:
      mode = SurvivalMode::EastHalf;
      break;
    case sf::Keyboard::Key::Num2:
      mode = SurvivalMode::WestHalf;
      break;
    case sf::Keyboard::Key::Num3:
      mode = SurvivalMode::QuadrantNE;
      break;
    case sf::Keyboard::Key::Num4:
      mode = SurvivalMode::QuadrantNW;
      break;
    case sf::Keyboard::Key::Num5:
      mode = SurvivalMode::QuadrantSE;
      break;
    case sf::Keyboard::Key::Num6:
      mode = SurvivalMode::QuadrantSW;
      break;
    case sf::Keyboard::Key::Num7:
      mode = SurvivalMode::FourCorners;
      break;
    default:
      break;
    }
  }
}
