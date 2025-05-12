#include "simulation.hpp"
#include <algorithm>
#include <cstring>
#include <random>

namespace {
std::mt19937 &globalRng() {
  static std::mt19937 rng{std::random_device{}()};
  return rng;
}
} // namespace

bool Simulation::isSurvivor(const Peep &p, SurvivalMode mode) const {
  const float half = WorldSize / 2.f;
  switch (mode) {
  case SurvivalMode::EastHalf:
    return p.position.x >= half;
  case SurvivalMode::WestHalf:
    return p.position.x < half;
  case SurvivalMode::QuadrantNE:
    return p.position.x >= half && p.position.y < half;
  case SurvivalMode::QuadrantNW:
    return p.position.x < half && p.position.y < half;
  case SurvivalMode::QuadrantSE:
    return p.position.x >= half && p.position.y >= half;
  case SurvivalMode::QuadrantSW:
    return p.position.x < half && p.position.y >= half;
  case SurvivalMode::FourCorners: {
    // define a small corner-square of size = WorldSize/4
    const float sz = WorldSize / 4.f;
    bool nw = p.position.x < sz && p.position.y < sz;
    bool ne = p.position.x >= WorldSize - sz && p.position.y < sz;
    bool sw = p.position.x < sz && p.position.y >= WorldSize - sz;
    bool se = p.position.x >= WorldSize - sz && p.position.y >= WorldSize - sz;
    return nw || ne || sw || se;
  }
  }
  return false;
}

static uint16_t encodeGene(Gene g) {
  // Clamp weight between -1.0 and 1.0 and convert to 10-bit integer
  float clamped = std::clamp(g.weight, -1.f, 1.f);
  int16_t fixedW = static_cast<int16_t>(std::round(clamped * 511.f));
  fixedW =
      std::clamp(fixedW, static_cast<int16_t>(-511), static_cast<int16_t>(511));

  // Pack into 16 bits: src (3 bits), dst (3 bits), weight (10 bits)
  uint16_t gene = 0;
  gene |= (uint16_t(g.src) & 0x07) << 13;
  gene |= (uint16_t(g.dst) & 0x07) << 10;
  gene |= (uint16_t(fixedW) & 0x03FF);
  return gene;
}

void Simulation::initPeeps() {
  std::uniform_real_distribution<float> posDist(0, WorldSize);
  std::uniform_real_distribution<float> wDist(-1.f, 1.f);
  std::uniform_int_distribution<int> idDist(0, 7);

  for (auto &p : peeps) {
    p.position = {posDist(globalRng()), posDist(globalRng())};
    for (auto &g : p.genome) {
      Gene gene{uint8_t(idDist(globalRng())), uint8_t(idDist(globalRng())),
                wDist(globalRng())};
      g = encodeGene(gene);
    }
    p.neurons.fill(0);
    p.age = 0;
  }
  generation = 0;
  timer = 0;
}

static Gene decodeGene(uint16_t raw) {
  Gene g;
  // extract 3-bit source and destination IDs
  g.src = (raw >> 13) & 0x07; // bits 15-13
  g.dst = (raw >> 10) & 0x07; // bits 12-10

  // extract 10-bit signed weight and sign-extend
  int16_t w = raw & 0x03FF; // bits 9-0
  if (w & 0x0200)           // if sign bit (bit 9) is set...
    w |= 0xFC00;            // …fill the high bits with 1s

  // convert back to float in [-1,1]
  g.weight = static_cast<float>(w) / 511.0f;
  return g;
}

void Simulation::update(float dt, bool speedUp, SurvivalMode mode) {
  timer += dt * (speedUp ? 10 : 1);
  if (timer >= GenerationTime) {
    // reproduce
    std::vector<Peep> survivors;
    for (const auto &p : peeps)
      if (isSurvivor(p, mode))
        survivors.push_back(p);
    if (survivors.empty())
      survivors.assign(peeps.begin(), peeps.end());

    std::uniform_real_distribution<float> posD(0, WorldSize);
    std::bernoulli_distribution mutD(MutationRate);
    std::uniform_int_distribution<int> bitD(0, 15);

    for (int i = 0; i < PeepCount; i++) {
      std::uniform_int_distribution<size_t> dist(0, survivors.size() - 1);
      const auto &parent = survivors[dist(globalRng())];
      Peep child = parent;
      child.position = {posD(globalRng()), posD(globalRng())};
      for (auto &gRaw : child.genome) {
        if (mutD(globalRng())) {
          gRaw ^= 1u << bitD(globalRng());
        }
      }
      peeps[i] = child;
    }
    generation++;
    timer = 0;
    return;
  }

  const float speed = 25.f * (speedUp ? 10.f : 1.f);
  for (auto &p : peeps) {
    p.age += dt * (speedUp ? 10 : 1);
    float lx = p.position.x / WorldSize;
    float ly = p.position.y / WorldSize;
    float mx = 0, my = 0;
    std::array<float, GenesPerPeep> newN = {};

    for (auto raw : p.genome) {
      Gene g = decodeGene(raw);
      float srcVal = (g.src == 0 ? lx : g.src == 1 ? ly : p.neurons[g.src - 2]);
      float contrib = srcVal * g.weight;
      if (g.dst == 6)
        mx += contrib;
      else if (g.dst == 7)
        my += contrib;
      else
        newN[g.dst - 2] += contrib;
    }
    for (int i = 0; i < GenesPerPeep; i++)
      p.neurons[i] = std::tanh(newN[i]);
    p.position.x = std::clamp(p.position.x + mx * speed, 0.f, WorldSize);
    p.position.y = std::clamp(p.position.y + my * speed, 0.f, WorldSize);
  }
}
