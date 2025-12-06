// peep_logic.cpp - Peep update and movement logic
// updatePeep, calculateInputs, movePeep, resolveCollisions

#include "../simulation.hpp"
#include "../core/constants.hpp"
#include <cmath>
#include <algorithm>

void Simulation::updatePeep(Peep& p, float dt) {
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

void Simulation::calculateInputs(const Peep& p, float* inputs) {
    for (int i = 0; i < NeuralConfig::INPUT_COUNT; i++) {
        inputs[i] = 0.0f;
    }
    
    inputs[NeuralConfig::IN_POS_X] = p.position.x / config.worldSize;
    inputs[NeuralConfig::IN_POS_Y] = p.position.y / config.worldSize;
    
    Food* food = findNearestFood(p.position);
    if (food && !food->eaten()) {
        sf::Vector2f fpos = food->position();
        float dx = fpos.x - p.position.x;
        float dy = fpos.y - p.position.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist > Constants::EPSILON) {
            inputs[NeuralConfig::IN_FOOD_DX] = dx / dist;
            inputs[NeuralConfig::IN_FOOD_DY] = dy / dist;
        }
        inputs[NeuralConfig::IN_FOOD_DIST] = std::min(1.0f, dist / config.peepSenseRange);
    } else {
        inputs[NeuralConfig::IN_FOOD_DIST] = 1.0f;
    }
    
    inputs[NeuralConfig::IN_WALL_DIST] = std::min(1.0f, distanceToWall(p.position) / config.peepSenseRange);
    
    inputs[NeuralConfig::IN_HUNGER] = config.hungerEnabled ? (p.hunger / config.maxHunger) : 1.0f;
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

void Simulation::movePeep(Peep& p, float mx, float my, float dt) {
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
        
        if (!insideObstacle(tryX)) newPos = tryX;
        else if (!insideObstacle(tryY)) newPos = tryY;
        else newPos = p.position;
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
        if (!peeps[i].alive) continue;
        
        for (int j = i + 1; j < n; j++) {
            if (!peeps[j].alive) continue;
            
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
