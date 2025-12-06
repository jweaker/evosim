// draw_world.cpp - World drawing methods
// Draws the simulation world: grid, peeps, food, obstacles, zones

#include "../render.hpp"
#include <algorithm>

void Renderer::drawWorld(sf::RenderWindow& window, Simulation& sim) {
    worldScale = worldDisplaySize / sim.config.worldSize;
    
    sf::RectangleShape glow({worldDisplaySize + 8, worldDisplaySize + 8});
    glow.setPosition({worldOffset.x - 4, worldOffset.y - 4});
    glow.setFillColor(sf::Color(40, 45, 55));
    glow.setOutlineColor(sf::Color(60, 65, 75));
    glow.setOutlineThickness(2);
    window.draw(glow);
    
    sf::RectangleShape bg({worldDisplaySize, worldDisplaySize});
    bg.setPosition(worldOffset);
    bg.setFillColor(sf::Color(20, 22, 28));
    window.draw(bg);
    
    drawGrid(window, sim);
    if (sim.config.spawnZonesEnabled) drawSpawnZones(window, sim);
    if (sim.config.survivalZoneEnabled) drawSurvivalZones(window, sim);
    if (sim.config.obstaclesEnabled) drawObstacles(window, sim);
    if (sim.config.hungerEnabled) drawFood(window, sim);
    drawPeeps(window, sim);
    
    sf::RectangleShape border({worldDisplaySize, worldDisplaySize});
    border.setPosition(worldOffset);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(80, 85, 95));
    border.setOutlineThickness(2);
    window.draw(border);
}

void Renderer::drawGrid(sf::RenderWindow& window, Simulation& sim) {
    (void)sim;  // Unused but kept for consistency
    
    float gridSize = 50.0f * worldScale;
    sf::Color gridColor(35, 38, 48);
    
    for (float px = gridSize; px < worldDisplaySize; px += gridSize) {
        sf::RectangleShape line({1.0f, worldDisplaySize});
        line.setPosition({worldOffset.x + px, worldOffset.y});
        line.setFillColor(gridColor);
        window.draw(line);
    }
    for (float py = gridSize; py < worldDisplaySize; py += gridSize) {
        sf::RectangleShape line({worldDisplaySize, 1.0f});
        line.setPosition({worldOffset.x, worldOffset.y + py});
        line.setFillColor(gridColor);
        window.draw(line);
    }
}

void Renderer::drawPeeps(sf::RenderWindow& window, Simulation& sim) {
    float radius = std::max(2.0f, sim.config.peepSize * worldScale);
    sf::CircleShape dot(radius);
    dot.setOrigin({radius, radius});
    
    for (size_t i = 0; i < sim.peeps.size(); i++) {
        const Peep& p = sim.peeps[i];
        if (!p.alive) continue;
        
        sf::Vector2f screenPos = worldOffset + p.position * worldScale;
        dot.setPosition(screenPos);
        
        sf::Color color;
        if (sim.config.showHungerColors && sim.config.hungerEnabled) {
            float ratio = std::max(0.0f, std::min(1.0f, p.hunger / sim.config.maxHunger));
            color = sf::Color((uint8_t)((1.0f - ratio) * 220 + 35), (uint8_t)(ratio * 200 + 55), 80);
        } else {
            color = sf::Color(p.colorR, p.colorG, p.colorB);
        }
        
        dot.setFillColor(color);
        
        if ((int)i == sim.selectedPeep) {
            dot.setOutlineColor(sf::Color(255, 220, 100));
            dot.setOutlineThickness(3);
        } else {
            dot.setOutlineThickness(0);
        }
        
        window.draw(dot);
        
        if (sim.config.showSenseRange) {
            float rr = sim.config.peepSenseRange * worldScale;
            sf::CircleShape range(rr);
            range.setOrigin({rr, rr});
            range.setPosition(screenPos);
            range.setFillColor(sf::Color(color.r, color.g, color.b, 20));
            range.setOutlineColor(sf::Color(color.r, color.g, color.b, 60));
            range.setOutlineThickness(1);
            window.draw(range);
        }
    }
}

void Renderer::drawFood(sf::RenderWindow& window, Simulation& sim) {
    float radius = std::max(2.0f, sim.config.foodSize * worldScale);
    sf::CircleShape dot(radius);
    dot.setOrigin({radius, radius});
    dot.setFillColor(sf::Color(80, 180, 100));
    dot.setOutlineColor(sf::Color(100, 220, 120, 100));
    dot.setOutlineThickness(2);
    
    for (const Food& f : sim.foods) {
        if (f.eaten()) continue;
        dot.setPosition(worldOffset + f.position() * worldScale);
        window.draw(dot);
    }
}

void Renderer::drawObstacles(sf::RenderWindow& window, Simulation& sim) {
    for (const Obstacle& obs : sim.obstacles) {
        sf::RectangleShape rect(obs.size() * worldScale);
        rect.setPosition(worldOffset + obs.position() * worldScale);
        rect.setFillColor(sf::Color(50, 52, 60));
        rect.setOutlineColor(sf::Color(70, 72, 80));
        rect.setOutlineThickness(1);
        window.draw(rect);
    }
}

void Renderer::drawSurvivalZones(sf::RenderWindow& window, Simulation& sim) {
    for (const SurvivalZone& z : sim.survivalZones) {
        sf::RectangleShape rect(z.size() * worldScale);
        rect.setPosition(worldOffset + z.position() * worldScale);
        rect.setFillColor(sf::Color(80, 180, 100, 50));
        rect.setOutlineColor(sf::Color(100, 220, 120, 150));
        rect.setOutlineThickness(2);
        window.draw(rect);
    }
    
    if (sim.survivalZones.empty()) {
        float half = sim.config.worldSize / 2.0f;
        int mode = sim.config.getSurvivalMode();
        
        sf::RectangleShape zone;
        zone.setFillColor(sf::Color(80, 180, 100, 40));
        zone.setOutlineColor(sf::Color(80, 180, 100, 80));
        zone.setOutlineThickness(1);
        
        if (mode == 0) {
            zone.setSize({half * worldScale, sim.config.worldSize * worldScale});
            zone.setPosition(worldOffset + sf::Vector2f(half * worldScale, 0));
            window.draw(zone);
        } else if (mode == 1) {
            zone.setSize({half * worldScale, sim.config.worldSize * worldScale});
            zone.setPosition(worldOffset);
            window.draw(zone);
        }
    }
}

void Renderer::drawSpawnZones(sf::RenderWindow& window, Simulation& sim) {
    for (const SpawnZone& z : sim.spawnZones) {
        sf::RectangleShape rect(z.size() * worldScale);
        rect.setPosition(worldOffset + z.position() * worldScale);
        rect.setFillColor(sf::Color(80, 150, 200, 50));
        rect.setOutlineColor(sf::Color(100, 180, 240, 150));
        rect.setOutlineThickness(2);
        window.draw(rect);
    }
    
    if (sim.spawnZones.empty() && sim.config.spawnOnLeft) {
        float sw = sim.config.worldSize * 0.2f;
        sf::RectangleShape zone({sw * worldScale, sim.config.worldSize * worldScale});
        zone.setPosition(worldOffset);
        zone.setFillColor(sf::Color(80, 150, 200, 30));
        zone.setOutlineColor(sf::Color(80, 150, 200, 60));
        zone.setOutlineThickness(1);
        window.draw(zone);
    }
}
