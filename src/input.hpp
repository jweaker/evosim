#pragma once
#include <SFML/Window/Event.hpp>

/// All the modes the user can pick:
enum class SurvivalMode {
  EastHalf,
  WestHalf,
  QuadrantNE,
  QuadrantNW,
  QuadrantSE,
  QuadrantSW,
  FourCorners
};

struct InputHandler {
  bool speedUp = false;
  SurvivalMode mode = SurvivalMode::EastHalf;
  bool paused = false;
  bool resetRequested = false;
  void processEvent(const sf::Event &e);
};
