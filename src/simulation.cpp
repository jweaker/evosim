#include "simulation.hpp"
#include "core/constants.hpp"
#include "neural/brain.hpp"
#include <algorithm>
#include <cmath>

Simulation::Simulation()
    : generationTimer(0.0f), currentGeneration(0), paused(false),
      speedMultiplier(1.0f), selectedPeep(-1), randomDist(-1.0f, 1.0f) {
  std::random_device rd;
  rng = std::mt19937(rd());
}

void Simulation::initialize() {
  peeps.clear();
  foods.clear();

  currentGeneration = 0;
  generationTimer = 0.0f;
  selectedPeep = -1;
  stats.reset();

  int pop = config.getInitialPopulation();
  peeps.reserve(pop);
  for (int i = 0; i < pop; i++) {
    peeps.push_back(createRandomPeep());
  }

  if (config.hungerEnabled) {
    spawnFood();
  }

  updateStats();
}

Peep Simulation::createRandomPeep() {
  Peep p;
  p.position = randomValidPosition();
  p.hunger = config.startingHunger;
  p.generation = currentGeneration;

  int genes = config.getGenesPerPeep();
  int hidden = config.getHiddenNeurons();

  p.genome = Brain::randomGenome(genes, rng);
  p.init(hidden, genes);
  p.updateColor();

  return p;
}

Peep Simulation::createChild(const Peep &parent) {
  Peep child;
  child.position = randomValidPosition();
  child.hunger = config.startingHunger;
  child.generation = currentGeneration;

  child.genome =
      Brain::mutate(parent.genome, config.mutationRate, config.mutationStrength,
                    rng, &child.mutatedBits);
  child.init(config.getHiddenNeurons(), config.getGenesPerPeep());
  child.updateColor();

  return child;
}

Peep Simulation::createChild(const Peep &p1, const Peep &p2) {
  Peep child;
  child.position = randomValidPosition();
  child.hunger = config.startingHunger;
  child.generation = currentGeneration;

  auto mixed = Brain::crossover(p1.genome, p2.genome, rng);
  child.genome =
      Brain::mutate(mixed, config.mutationRate, config.mutationStrength, rng,
                    &child.mutatedBits);
  child.init(config.getHiddenNeurons(), config.getGenesPerPeep());
  child.updateColor();

  return child;
}

void Simulation::update(float deltaTime) {
  if (paused)
    return;

  float simTime = deltaTime * speedMultiplier;

  while (simTime > 0.0f) {
    float step = std::min(simTime, Constants::MAX_PHYSICS_STEP);
    simTime -= step;

    generationTimer += step;

    if (config.hungerEnabled) {
      updateFood(step);
    }

    int alive = 0;
    for (auto &p : peeps) {
      if (!p.alive)
        continue;
      updatePeep(p, step);
      if (p.alive)
        alive++;
    }

    resolveCollisions();

    bool shouldEnd = (generationTimer >= config.generationTime);

    if (config.hungerEnabled) {
      int minPop = config.getMinPopulation();
      if (alive <= minPop && alive > 0) {
        shouldEnd = true;
      } else if (alive == 0) {
        initialize();
        return;
      }
    }

    if (shouldEnd) {
      endGeneration();
      break;
    }
  }

  updateStats();
}

void Simulation::updatePeep(Peep &p, float dt) {
  p.age += dt;

  if (config.hungerEnabled) {
    p.hunger -= config.hungerDecayRate * dt;
    if (p.hunger <= 0) {
      p.alive = false;
      return;
    }
  }

  float inputs[NeuralConfig::INPUT_COUNT];
  calculateInputs(p, inputs);

  p.brain.process(inputs, NeuralConfig::INPUT_COUNT);

  float mx = p.brain.getOutput(NeuralConfig::OUT_MOVE_X);
  float my = p.brain.getOutput(NeuralConfig::OUT_MOVE_Y);
  movePeep(p, mx, my, dt);

  if (config.hungerEnabled) {
    checkFood(p);
  }
}

void Simulation::calculateInputs(const Peep &p, float *inputs) {
  for (int i = 0; i < NeuralConfig::INPUT_COUNT; i++) {
    inputs[i] = 0.0f;
  }

  inputs[NeuralConfig::IN_POS_X] = p.position.x / config.worldSize;
  inputs[NeuralConfig::IN_POS_Y] = p.position.y / config.worldSize;

  Food *food = findNearestFood(p.position);
  if (food && !food->eaten()) {
    sf::Vector2f fpos = food->position();
    float dx = fpos.x - p.position.x;
    float dy = fpos.y - p.position.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > Constants::EPSILON) {
      inputs[NeuralConfig::IN_FOOD_DX] = dx / dist;
      inputs[NeuralConfig::IN_FOOD_DY] = dy / dist;
    }
    inputs[NeuralConfig::IN_FOOD_DIST] =
        std::min(1.0f, dist / config.peepSenseRange);
  } else {
    inputs[NeuralConfig::IN_FOOD_DIST] = 1.0f;
  }

  inputs[NeuralConfig::IN_WALL_DIST] =
      std::min(1.0f, distanceToWall(p.position) / config.peepSenseRange);

  inputs[NeuralConfig::IN_HUNGER] =
      config.hungerEnabled ? (p.hunger / config.maxHunger) : 1.0f;
  inputs[NeuralConfig::IN_AGE] = std::min(1.0f, p.age / config.generationTime);

  float timeLeft = 1.0f - (generationTimer / config.generationTime);
  inputs[NeuralConfig::IN_TIME_LEFT] = std::max(0.0f, std::min(1.0f, timeLeft));

  float zDx, zDy, zDist;
  bool inZone;
  findNearestZone(p.position, zDx, zDy, zDist, inZone);
  inputs[NeuralConfig::IN_ZONE_DX] = zDx;
  inputs[NeuralConfig::IN_ZONE_DY] = zDy;
  inputs[NeuralConfig::IN_ZONE_DIST] = zDist;
  inputs[NeuralConfig::IN_ZONE] = inZone ? 1.0f : 0.0f;

  inputs[NeuralConfig::IN_RANDOM] = randomDist(rng);
  inputs[NeuralConfig::IN_BIAS] = 1.0f;
}

void Simulation::movePeep(Peep &p, float mx, float my, float dt) {
  float speed = config.peepSpeed * dt;
  sf::Vector2f newPos = p.position;
  newPos.x += mx * speed;
  newPos.y += my * speed;

  float maxP = config.worldSize - Constants::WORLD_MARGIN;
  newPos.x = std::max(Constants::WORLD_MARGIN, std::min(maxP, newPos.x));
  newPos.y = std::max(Constants::WORLD_MARGIN, std::min(maxP, newPos.y));

  if (config.obstaclesEnabled && insideObstacle(newPos)) {
    sf::Vector2f tryX(newPos.x, p.position.y);
    sf::Vector2f tryY(p.position.x, newPos.y);

    if (!insideObstacle(tryX))
      newPos = tryX;
    else if (!insideObstacle(tryY))
      newPos = tryY;
    else
      newPos = p.position;
  }

  float dx = newPos.x - p.position.x;
  float dy = newPos.y - p.position.y;
  float moved = std::sqrt(dx * dx + dy * dy);

  if (config.hungerEnabled && moved > Constants::EPSILON) {
    p.hunger -= config.movementHungerCost * moved;
  }

  p.position = newPos;
  if (dt > Constants::EPSILON) {
    p.velocity = {dx / dt, dy / dt};
  }
}

void Simulation::resolveCollisions() {
  float radius = config.peepSize * 2.0f;
  float radiusSq = radius * radius;
  float maxP = config.worldSize - Constants::WORLD_MARGIN;

  int n = (int)peeps.size();
  for (int i = 0; i < n; i++) {
    if (!peeps[i].alive)
      continue;

    for (int j = i + 1; j < n; j++) {
      if (!peeps[j].alive)
        continue;

      float dx = peeps[j].position.x - peeps[i].position.x;
      float dy = peeps[j].position.y - peeps[i].position.y;
      float distSq = dx * dx + dy * dy;

      if (distSq < radiusSq && distSq > Constants::EPSILON) {
        float dist = std::sqrt(distSq);
        float overlap = radius - dist;
        float nx = dx / dist;
        float ny = dy / dist;
        float push = overlap * 0.5f;

        sf::Vector2f newI = peeps[i].position;
        newI.x -= nx * push;
        newI.y -= ny * push;
        newI.x = std::max(Constants::WORLD_MARGIN, std::min(maxP, newI.x));
        newI.y = std::max(Constants::WORLD_MARGIN, std::min(maxP, newI.y));

        sf::Vector2f newJ = peeps[j].position;
        newJ.x += nx * push;
        newJ.y += ny * push;
        newJ.x = std::max(Constants::WORLD_MARGIN, std::min(maxP, newJ.x));
        newJ.y = std::max(Constants::WORLD_MARGIN, std::min(maxP, newJ.y));

        bool iBlocked = config.obstaclesEnabled && insideObstacle(newI);
        bool jBlocked = config.obstaclesEnabled && insideObstacle(newJ);

        if (!iBlocked && !jBlocked) {
          peeps[i].position = newI;
          peeps[j].position = newJ;
        } else if (!iBlocked) {
          peeps[i].position.x -= nx * overlap;
          peeps[i].position.y -= ny * overlap;
        } else if (!jBlocked) {
          peeps[j].position.x += nx * overlap;
          peeps[j].position.y += ny * overlap;
        }
      }
    }
  }
}

void Simulation::checkFood(Peep &p) {
  float eatDist = config.peepSize + config.foodSize;

  for (auto &f : foods) {
    if (f.eaten())
      continue;

    sf::Vector2f fpos = f.position();
    float dx = fpos.x - p.position.x;
    float dy = fpos.y - p.position.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < eatDist) {
      f.eat(config.foodRespawnTime);
      p.hunger = std::min(config.maxHunger, p.hunger + config.foodHunger);
    }
  }
}

void Simulation::updateFood(float dt) {
  for (auto &f : foods) {
    if (!f.eaten())
      continue;
    f.update(dt);
    if (f.respawnTimer() <= 0) {
      f.respawn(randomValidPosition());
    }
  }
}

void Simulation::endGeneration() {
  auto survivors = getSurvivors();

  float rate = 0.0f;
  if (!peeps.empty()) {
    if (config.hungerEnabled) {
      int alive = 0;
      for (auto &p : peeps)
        if (p.alive)
          alive++;
      rate = (float)alive / (float)peeps.size();
    } else {
      rate = (float)survivors.size() / (float)peeps.size();
    }
  }

  float avgHunger = 0.0f;
  for (auto *s : survivors)
    avgHunger += s->hunger;
  if (!survivors.empty())
    avgHunger /= (float)survivors.size();

  stats.record(rate, (int)survivors.size(), avgHunger);

  if (survivors.empty()) {
    initialize();
    return;
  }

  std::vector<Peep> nextGen;
  int target = config.getInitialPopulation();
  nextGen.reserve(target);

  std::uniform_int_distribution<int> parentDist(0, (int)survivors.size() - 1);
  std::bernoulli_distribution crossDist(config.crossoverRate);

  for (int i = 0; i < target; i++) {
    bool useTwoParents =
        config.useCrossover && survivors.size() > 1 && crossDist(rng);

    if (useTwoParents) {
      int idx1 = parentDist(rng);
      int idx2 = parentDist(rng);
      for (int a = 0; a < 10 && idx2 == idx1; a++) {
        idx2 = parentDist(rng);
      }
      nextGen.push_back(createChild(*survivors[idx1], *survivors[idx2]));
    } else {
      nextGen.push_back(createChild(*survivors[parentDist(rng)]));
    }
  }

  peeps = std::move(nextGen);

  if (config.hungerEnabled) {
    spawnFood();
  }

  currentGeneration++;
  generationTimer = 0.0f;
  selectedPeep = -1;
  updateStats();
}

std::vector<Peep *> Simulation::getSurvivors() {
  std::vector<Peep *> survivors;
  bool useDrawn = config.survivalZoneEnabled && !survivalZones.empty();

  for (auto &p : peeps) {
    if (!p.alive)
      continue;

    bool survives = true;

    if (useDrawn) {
      survives = inSurvivalZone(p.position);
    } else if (config.survivalZoneEnabled) {
      float half = config.worldSize / 2.0f;
      float quarter = config.worldSize / 4.0f;
      int mode = config.getSurvivalMode();

      switch (mode) {
      case 0:
        survives = (p.position.x >= half);
        break;
      case 1:
        survives = (p.position.x < half);
        break;
      case 2:
        survives = (p.position.x >= half && p.position.y < half);
        break;
      case 3:
        survives = (p.position.x < half && p.position.y < half);
        break;
      case 4:
        survives = (p.position.x >= half && p.position.y >= half);
        break;
      case 5:
        survives = (p.position.x < half && p.position.y >= half);
        break;
      case 6:
        survives = (p.position.x < quarter && p.position.y < quarter) ||
                   (p.position.x >= config.worldSize - quarter &&
                    p.position.y < quarter) ||
                   (p.position.x < quarter &&
                    p.position.y >= config.worldSize - quarter) ||
                   (p.position.x >= config.worldSize - quarter &&
                    p.position.y >= config.worldSize - quarter);
        break;
      }
    }

    if (survives)
      survivors.push_back(&p);
  }

  return survivors;
}

Food *Simulation::findNearestFood(sf::Vector2f pos) {
  Food *nearest = nullptr;
  float minDistSq = 1e9f;

  for (auto &f : foods) {
    if (f.eaten())
      continue;

    sf::Vector2f fpos = f.position();
    float dx = fpos.x - pos.x;
    float dy = fpos.y - pos.y;
    float distSq = dx * dx + dy * dy;

    if (distSq < minDistSq) {
      minDistSq = distSq;
      nearest = &f;
    }
  }
  return nearest;
}

float Simulation::distanceToWall(sf::Vector2f pos) {
  float minD = std::min(
      {pos.x, pos.y, config.worldSize - pos.x, config.worldSize - pos.y});

  for (auto &obs : obstacles) {
    sf::Vector2f closest = obs.closestPoint(pos);
    float dx = closest.x - pos.x;
    float dy = closest.y - pos.y;
    float d = std::sqrt(dx * dx + dy * dy);
    if (d < minD)
      minD = d;
  }
  return minD;
}

bool Simulation::insideObstacle(sf::Vector2f pos) {
  for (auto &obs : obstacles) {
    if (obs.contains(pos))
      return true;
  }
  return false;
}

bool Simulation::inSurvivalZone(sf::Vector2f pos) {
  for (auto &z : survivalZones) {
    if (z.contains(pos))
      return true;
  }
  return false;
}

void Simulation::findNearestZone(sf::Vector2f pos, float &dx, float &dy,
                                 float &dist, bool &inside) {
  dx = dy = 0.0f;
  dist = 1.0f;
  inside = false;

  if (survivalZones.empty()) {
    if (!config.survivalZoneEnabled)
      return;

    float half = config.worldSize / 2.0f;
    int mode = config.getSurvivalMode();

    sf::Vector2f target;
    if (mode == 0) {
      target = {config.worldSize * 0.75f, config.worldSize * 0.5f};
      inside = (pos.x >= half);
    } else if (mode == 1) {
      target = {config.worldSize * 0.25f, config.worldSize * 0.5f};
      inside = (pos.x < half);
    } else {
      target = {config.worldSize * 0.75f, config.worldSize * 0.5f};
      inside = (pos.x >= half);
    }

    float ddx = target.x - pos.x;
    float ddy = target.y - pos.y;
    float d = std::sqrt(ddx * ddx + ddy * ddy);

    if (d > Constants::EPSILON) {
      dx = ddx / d;
      dy = ddy / d;
    }
    dist = inside ? 0.0f : std::min(1.0f, d / config.worldSize);
    return;
  }

  float minD = 1e9f;
  sf::Vector2f nearest;

  for (auto &zone : survivalZones) {
    if (zone.contains(pos)) {
      inside = true;
      dist = 0.0f;
      sf::Vector2f c = zone.center();
      float ddx = c.x - pos.x;
      float ddy = c.y - pos.y;
      float d = std::sqrt(ddx * ddx + ddy * ddy);
      if (d > Constants::EPSILON) {
        dx = ddx / d;
        dy = ddy / d;
      }
      return;
    }

    sf::Vector2f closest = zone.closestPoint(pos);
    float ddx = closest.x - pos.x;
    float ddy = closest.y - pos.y;
    float d = std::sqrt(ddx * ddx + ddy * ddy);

    if (d < minD) {
      minD = d;
      nearest = closest;
    }
  }

  float ddx = nearest.x - pos.x;
  float ddy = nearest.y - pos.y;
  float d = std::sqrt(ddx * ddx + ddy * ddy);

  if (d > Constants::EPSILON) {
    dx = ddx / d;
    dy = ddy / d;
  }
  dist = std::min(1.0f, minD / config.peepSenseRange);
}

sf::Vector2f Simulation::randomValidPosition() {
  if (config.spawnZonesEnabled && !spawnZones.empty()) {
    std::uniform_int_distribution<int> zoneDist(0, (int)spawnZones.size() - 1);

    for (int i = 0; i < Constants::MAX_POSITION_ATTEMPTS; i++) {
      sf::Vector2f pos = spawnZones[zoneDist(rng)].randomPoint(rng);
      if (!insideObstacle(pos))
        return pos;
    }
    return spawnZones[0].randomPoint(rng);
  }

  float minX = Constants::WORLD_MARGIN;
  float maxX = config.worldSize - Constants::WORLD_MARGIN;

  if (config.spawnZonesEnabled && config.spawnOnLeft) {
    maxX = config.worldSize * 0.2f;
  }

  std::uniform_real_distribution<float> distX(minX, maxX);
  std::uniform_real_distribution<float> distY(
      Constants::WORLD_MARGIN, config.worldSize - Constants::WORLD_MARGIN);

  for (int i = 0; i < Constants::MAX_POSITION_ATTEMPTS; i++) {
    sf::Vector2f pos(distX(rng), distY(rng));
    if (!insideObstacle(pos))
      return pos;
  }
  return {distX(rng), distY(rng)};
}

void Simulation::spawnFood() {
  foods.clear();
  int count = config.getFoodCount();
  foods.reserve(count);
  for (int i = 0; i < count; i++) {
    sf::Vector2f pos = randomValidPosition();
    foods.emplace_back(pos.x, pos.y);
  }
}

void Simulation::updateStats() {
  stats.currentGeneration = currentGeneration;
  stats.populationCount = (int)peeps.size();

  int alive = 0;
  float hunger = 0.0f;
  for (auto &p : peeps) {
    if (p.alive) {
      alive++;
      hunger += p.hunger;
    }
  }

  stats.aliveCount = alive;
  stats.averageHunger = alive > 0 ? hunger / (float)alive : 0.0f;
  stats.survivalRate =
      peeps.empty() ? 0.0f : (float)alive / (float)peeps.size();
}

void Simulation::reset() { initialize(); }

void Simulation::applyPreset(int index) {
  obstacles.clear();
  survivalZones.clear();
  spawnZones.clear();

  switch (index) {
  case 0:
    config.presetMigration();
    break;
  case 1:
    config.presetForaging();
    break;
  case 2:
    config.presetMazeRunner();
    generateMazeObstacles();
    break;
  case 3:
    config.presetHungerGames();
    generateMazeObstacles();
    break;
  case 4:
    config.presetBigBrains();
    generateMazeObstacles();
    break;
  case 5:
    config.presetSpeedDemons();
    break;
  default:
    config.resetToDefaults();
    break;
  }

  if (config.survivalZoneEnabled) {
    generatePresetZone();
  }
  if (config.spawnOnLeft) {
    generatePresetSpawnZone();
  }

  initialize();
}

void Simulation::generateMazeObstacles() {
  obstacles.clear();
  float ws = config.worldSize;
  float thick = 30.0f;
  float gap = 80.0f;

  obstacles.emplace_back(ws * 0.25f - thick / 2, gap, thick, ws - gap);
  obstacles.emplace_back(ws * 0.5f - thick / 2, 0, thick, ws - gap);
  obstacles.emplace_back(ws * 0.75f - thick / 2, 0, thick, ws * 0.4f);
  obstacles.emplace_back(ws * 0.75f - thick / 2, ws * 0.6f, thick, ws * 0.4f);
}

void Simulation::generatePresetZone() {
  survivalZones.clear();
  float ws = config.worldSize;
  int mode = config.getSurvivalMode();

  switch (mode) {
  case 0:
    survivalZones.emplace_back(ws * 0.8f, 0, ws * 0.2f, ws);
    break;
  case 1:
    survivalZones.emplace_back(0, 0, ws * 0.2f, ws);
    break;
  case 2:
    survivalZones.emplace_back(ws * 0.3f, ws * 0.3f, ws * 0.4f, ws * 0.4f);
    break;
  case 3:
    survivalZones.emplace_back(0, 0, ws * 0.15f, ws * 0.15f);
    survivalZones.emplace_back(ws * 0.85f, 0, ws * 0.15f, ws * 0.15f);
    survivalZones.emplace_back(0, ws * 0.85f, ws * 0.15f, ws * 0.15f);
    survivalZones.emplace_back(ws * 0.85f, ws * 0.85f, ws * 0.15f, ws * 0.15f);
    break;
  default:
    survivalZones.emplace_back(ws * 0.8f, 0, ws * 0.2f, ws);
    break;
  }
}

void Simulation::generatePresetSpawnZone() {
  spawnZones.clear();
  if (config.spawnOnLeft) {
    float ws = config.worldSize;
    float margin = Constants::WORLD_MARGIN;
    spawnZones.emplace_back(margin, margin, ws * 0.2f - margin,
                            ws - margin * 2);
  }
}
