#include "color_utils.hpp"
#include <cmath>

sf::Color hslToRgb(float h, float s, float l) {
  h = std::fmod(h, 360.f);
  float c = (1 - std::abs(2 * l - 1)) * s;
  float x = c * (1 - std::abs(std::fmod(h / 60.f, 2) - 1));
  float m = l - c / 2;
  float r = 0, g = 0, b = 0;
  if (h < 60) {
    r = c;
    g = x;
  } else if (h < 120) {
    r = x;
    g = c;
  } else if (h < 180) {
    g = c;
    b = x;
  } else if (h < 240) {
    g = x;
    b = c;
  } else if (h < 300) {
    r = x;
    b = c;
  } else {
    r = c;
    b = x;
  }
  return sf::Color((r + m) * 255, (g + m) * 255, (b + m) * 255);
}

sf::Color genomeToColor(const std::array<uint16_t, 4> &genome) {
  uint32_t sum = 0;
  for (auto g : genome)
    sum += g;
  return hslToRgb(sum % 360, 0.8f, 0.6f);
}
