// input.cpp - Input handling and main loop
// Mouse clicks, keyboard input, event handling, main loop

#include "../render.hpp"
#include <cmath>
#include <algorithm>

void Renderer::handleEvent(const sf::Event& event, sf::RenderWindow& window, Simulation& sim) {
    if (!uiReady) setupUI(sim);
    
    if (const auto* press = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f pos((float)press->position.x, (float)press->position.y);
        handleMouseClick(pos, press->button, sim);
    }
    else if (const auto* release = event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::Vector2f pos((float)release->position.x, (float)release->position.y);
        handleMouseRelease(pos, sim);
    }
    else if (const auto* move = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f pos((float)move->position.x, (float)move->position.y);
        handleMouseMove(pos, sim);
    }
    else if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        handleKeyPress(key->code, sim);
    }
    else if (event.is<sf::Event::Resized>()) {
        sf::Vector2u sz = window.getSize();
        sf::View view(sf::FloatRect({0, 0}, {(float)sz.x, (float)sz.y}));
        window.setView(view);
        windowWidth = (int)sz.x;
        windowHeight = (int)sz.y;
        recalculateLayout();
        uiReady = false;
    }
}

void Renderer::handleMouseClick(sf::Vector2f pos, sf::Mouse::Button btn, Simulation& sim) {
    sf::FloatRect world(worldOffset, {worldDisplaySize, worldDisplaySize});
    
    auto handleEdit = [&](auto& items, bool& editing) {
        if (editing && world.contains(pos)) {
            sf::Vector2f wp = (pos - worldOffset) / worldScale;
            
            if (btn == sf::Mouse::Button::Right) {
                for (auto it = items.begin(); it != items.end();) {
                    if (it->contains(wp)) it = items.erase(it);
                    else ++it;
                }
                return true;
            } else if (btn == sf::Mouse::Button::Left) {
                isDrawing = true;
                isErasing = false;
                drawStart = wp;
                return true;
            }
        }
        return false;
    };
    
    if (handleEdit(sim.obstacles, editingObstacles)) return;
    if (handleEdit(sim.survivalZones, editingSurvivalZones)) return;
    if (handleEdit(sim.spawnZones, editingSpawnZones)) return;
    
    if (btn != sf::Mouse::Button::Left) return;
    
    for (size_t i = 0; i < buttons.size(); i++) {
        if (buttons[i].handleClick(pos)) {
            if (i == 0) sim.paused = !sim.paused;
            else if (i == 1) { sim.reset(); uiReady = false; }
            else if (i == 2) { sim.config.resetToDefaults(); uiReady = false; }
            else if (i == 3) { editingObstacles = !editingObstacles; editingSurvivalZones = editingSpawnZones = false; if (editingObstacles) sim.config.obstaclesEnabled = true; }
            else if (i == 4) { editingSurvivalZones = !editingSurvivalZones; editingObstacles = editingSpawnZones = false; if (editingSurvivalZones) sim.config.survivalZoneEnabled = true; }
            else if (i == 5) { editingSpawnZones = !editingSpawnZones; editingObstacles = editingSurvivalZones = false; if (editingSpawnZones) sim.config.spawnZonesEnabled = true; }
            else if (i >= 6 && i <= 11) { sim.applyPreset((int)(i - 6)); uiReady = false; }
            return;
        }
    }
    
    for (size_t i = 0; i < sliders.size(); i++) {
        if (sliders[i].handleClick(pos)) {
            draggingSlider = true;
            activeSlider = (int)i;
            return;
        }
    }
    
    for (auto& t : toggles) if (t.handleClick(pos)) return;
    
    if (bottomToggles[0].contains(pos)) { sim.config.showSenseRange = !sim.config.showSenseRange; return; }
    if (bottomToggles[1].contains(pos)) { sim.config.showHungerColors = !sim.config.showHungerColors; return; }
    
    if (!editingObstacles && !editingSurvivalZones && !editingSpawnZones && world.contains(pos)) {
        sf::Vector2f wp = (pos - worldOffset) / worldScale;
        float minD = sim.config.peepSize * 4;
        int closest = -1;
        
        for (size_t i = 0; i < sim.peeps.size(); i++) {
            if (!sim.peeps[i].alive) continue;
            sf::Vector2f diff = sim.peeps[i].position - wp;
            float d = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            if (d < minD) { minD = d; closest = (int)i; }
        }
        sim.selectedPeep = closest;
    }
}

void Renderer::handleMouseMove(sf::Vector2f pos, Simulation& sim) {
    (void)sim;
    if (draggingSlider && activeSlider >= 0 && activeSlider < (int)sliders.size()) {
        sliders[activeSlider].handleDrag(pos);
    }
}

void Renderer::handleMouseRelease(sf::Vector2f pos, Simulation& sim) {
    auto finishDraw = [&](auto& items) {
        sf::Vector2f we = (pos - worldOffset) / worldScale;
        float x = std::min(drawStart.x, we.x);
        float y = std::min(drawStart.y, we.y);
        float w = std::abs(we.x - drawStart.x);
        float h = std::abs(we.y - drawStart.y);
        
        if (w > 10 && h > 10) {
            x = std::max(0.0f, std::min(x, sim.config.worldSize - w));
            y = std::max(0.0f, std::min(y, sim.config.worldSize - h));
            items.emplace_back(x, y, w, h);
        }
        isDrawing = false;
    };
    
    if (isDrawing && editingObstacles) finishDraw(sim.obstacles);
    else if (isDrawing && editingSurvivalZones) finishDraw(sim.survivalZones);
    else if (isDrawing && editingSpawnZones) finishDraw(sim.spawnZones);
    
    if (draggingSlider && activeSlider >= 0 && activeSlider < (int)sliders.size()) {
        sliders[activeSlider].dragging = false;
    }
    draggingSlider = false;
    activeSlider = -1;
}

void Renderer::handleKeyPress(sf::Keyboard::Key key, Simulation& sim) {
    switch (key) {
        case sf::Keyboard::Key::Space: sim.paused = !sim.paused; break;
        case sf::Keyboard::Key::R: sim.reset(); uiReady = false; break;
        case sf::Keyboard::Key::E:
            editingObstacles = !editingObstacles;
            editingSurvivalZones = editingSpawnZones = false;
            if (editingObstacles) sim.config.obstaclesEnabled = true;
            isDrawing = false;
            break;
        case sf::Keyboard::Key::Z:
            editingSurvivalZones = !editingSurvivalZones;
            editingObstacles = editingSpawnZones = false;
            if (editingSurvivalZones) sim.config.survivalZoneEnabled = true;
            isDrawing = false;
            break;
        case sf::Keyboard::Key::S:
            editingSpawnZones = !editingSpawnZones;
            editingObstacles = editingSurvivalZones = false;
            if (editingSpawnZones) sim.config.spawnZonesEnabled = true;
            isDrawing = false;
            break;
        case sf::Keyboard::Key::Escape:
            if (editingObstacles || editingSurvivalZones || editingSpawnZones) {
                editingObstacles = editingSurvivalZones = editingSpawnZones = false;
                isDrawing = false;
            } else {
                sim.selectedPeep = -1;
            }
            break;
        case sf::Keyboard::Key::Up: sim.speedMultiplier = std::min(100.0f, sim.speedMultiplier < 10 ? sim.speedMultiplier + 1 : sim.speedMultiplier * 1.5f); break;
        case sf::Keyboard::Key::Down: sim.speedMultiplier = std::max(1.0f, sim.speedMultiplier > 10 ? sim.speedMultiplier / 1.5f : sim.speedMultiplier - 1); break;
        case sf::Keyboard::Key::Num1: sim.speedMultiplier = 1.0f; break;
        case sf::Keyboard::Key::Num2: sim.speedMultiplier = 2.0f; break;
        case sf::Keyboard::Key::Num5: sim.speedMultiplier = 5.0f; break;
        case sf::Keyboard::Key::Num0: sim.speedMultiplier = 10.0f; break;
        default: break;
    }
}

void runMainLoop(sf::RenderWindow& window, Simulation& sim, Renderer& renderer) {
    sf::Clock clock;
    
    while (window.isOpen()) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
                break;
            }
            renderer.handleEvent(*event, window, sim);
        }
        
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;
        
        sim.update(dt);
        renderer.render(window, sim);
    }
}
