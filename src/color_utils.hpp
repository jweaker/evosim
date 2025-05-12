
#pragma once
#include <SFML/Graphics.hpp>

sf::Color hslToRgb(float h, float s, float l);
sf::Color genomeToColor(const std::array<uint16_t, 4> &genome);
