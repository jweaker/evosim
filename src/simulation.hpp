#pragma once
#include "input.hpp"
#include <SFML/System/Vector2.hpp>
#include <array>
#include <cstdint>

constexpr int PeepCount = 512;
constexpr float WorldSize = 512.f;
constexpr int GenesPerPeep = 4;
constexpr float GenerationTime = 10.f;
constexpr float MutationRate = 0.01f;

struct Gene {
  uint8_t src, dst;
  float weight;
};
struct Peep {
  sf::Vector2f position;
  std::array<uint16_t, GenesPerPeep> genome;
  std::array<float, GenesPerPeep> neurons{};
  float age = 0;
};

struct Simulation {
  std::array<Peep, PeepCount> peeps;
  int generation = 0;
  float timer = 0;

  void initPeeps();
  void update(float dt, bool speedUp, SurvivalMode mode);
  bool isSurvivor(const Peep &p, SurvivalMode mode) const;
};
