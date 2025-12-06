// world.cpp - World helper functions
// Food, obstacles, zones, position generation, maze generation

#include "../simulation.hpp"
#include "../core/constants.hpp"
#include <cmath>
#include <algorithm>

void Simulation::checkFood(Peep& p) {
    float eatDist = config.peepSize + config.foodSize;
    
    for (auto& f : foods) {
        if (f.eaten()) continue;
        
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
    for (auto& f : foods) {
        if (!f.eaten()) continue;
        f.update(dt);
        if (f.respawnTimer() <= 0) {
            f.respawn(randomValidPosition());
        }
    }
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

Food* Simulation::findNearestFood(sf::Vector2f pos) {
    Food* nearest = nullptr;
    float minDistSq = 1e9f;
    
    for (auto& f : foods) {
        if (f.eaten()) continue;
        
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
    float minD = std::min({pos.x, pos.y, config.worldSize - pos.x, config.worldSize - pos.y});
    
    for (auto& obs : obstacles) {
        sf::Vector2f closest = obs.closestPoint(pos);
        float dx = closest.x - pos.x;
        float dy = closest.y - pos.y;
        float d = std::sqrt(dx * dx + dy * dy);
        if (d < minD) minD = d;
    }
    return minD;
}

bool Simulation::insideObstacle(sf::Vector2f pos) {
    for (auto& obs : obstacles) {
        if (obs.contains(pos)) return true;
    }
    return false;
}

bool Simulation::inSurvivalZone(sf::Vector2f pos) {
    for (auto& z : survivalZones) {
        if (z.contains(pos)) return true;
    }
    return false;
}

void Simulation::findNearestZone(sf::Vector2f pos, float& dx, float& dy, float& dist, bool& inside) {
    dx = dy = 0.0f;
    dist = 1.0f;
    inside = false;
    
    if (survivalZones.empty()) {
        if (!config.survivalZoneEnabled) return;
        
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
    
    for (auto& zone : survivalZones) {
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
            if (!insideObstacle(pos)) return pos;
        }
        return spawnZones[0].randomPoint(rng);
    }
    
    float minX = Constants::WORLD_MARGIN;
    float maxX = config.worldSize - Constants::WORLD_MARGIN;
    
    if (config.spawnZonesEnabled && config.spawnOnLeft) {
        maxX = config.worldSize * 0.2f;
    }
    
    std::uniform_real_distribution<float> distX(minX, maxX);
    std::uniform_real_distribution<float> distY(Constants::WORLD_MARGIN, config.worldSize - Constants::WORLD_MARGIN);
    
    for (int i = 0; i < Constants::MAX_POSITION_ATTEMPTS; i++) {
        sf::Vector2f pos(distX(rng), distY(rng));
        if (!insideObstacle(pos)) return pos;
    }
    return {distX(rng), distY(rng)};
}

void Simulation::generateMazeObstacles() {
    obstacles.clear();
    float ws = config.worldSize;
    float thick = 30.0f;
    float gap = 80.0f;
    
    obstacles.emplace_back(ws * 0.25f - thick/2, gap, thick, ws - gap);
    obstacles.emplace_back(ws * 0.5f - thick/2, 0, thick, ws - gap);
    obstacles.emplace_back(ws * 0.75f - thick/2, 0, thick, ws * 0.4f);
    obstacles.emplace_back(ws * 0.75f - thick/2, ws * 0.6f, thick, ws * 0.4f);
}

void Simulation::generatePresetZone() {
    survivalZones.clear();
    float ws = config.worldSize;
    int mode = config.getSurvivalMode();
    
    switch (mode) {
        case 0: survivalZones.emplace_back(ws * 0.8f, 0, ws * 0.2f, ws); break;
        case 1: survivalZones.emplace_back(0, 0, ws * 0.2f, ws); break;
        case 2: survivalZones.emplace_back(ws * 0.3f, ws * 0.3f, ws * 0.4f, ws * 0.4f); break;
        case 3:
            survivalZones.emplace_back(0, 0, ws * 0.15f, ws * 0.15f);
            survivalZones.emplace_back(ws * 0.85f, 0, ws * 0.15f, ws * 0.15f);
            survivalZones.emplace_back(0, ws * 0.85f, ws * 0.15f, ws * 0.15f);
            survivalZones.emplace_back(ws * 0.85f, ws * 0.85f, ws * 0.15f, ws * 0.15f);
            break;
        default: survivalZones.emplace_back(ws * 0.8f, 0, ws * 0.2f, ws); break;
    }
}

void Simulation::generatePresetSpawnZone() {
    spawnZones.clear();
    if (config.spawnOnLeft) {
        float ws = config.worldSize;
        float margin = Constants::WORLD_MARGIN;
        spawnZones.emplace_back(margin, margin, ws * 0.2f - margin, ws - margin * 2);
    }
}
