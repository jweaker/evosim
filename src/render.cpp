// render.cpp - Rendering and UI implementation

#include "render.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

// ============================================================================
// UI HELPER IMPLEMENTATIONS
// ============================================================================

Slider::Slider() 
    : minValue(0), maxValue(1), value(nullptr), isInteger(false), isLogarithmic(false), dragging(false) {}

Slider::Slider(float x, float y, float width, float height,
               float minVal, float maxVal, float* val,
               const std::string& lbl, bool isInt, bool isLog)
    : bounds({x, y}, {width, height})
    , minValue(minVal), maxValue(maxVal)
    , value(val), label(lbl), isInteger(isInt), isLogarithmic(isLog), dragging(false) {}

bool Slider::handleClick(sf::Vector2f mousePos) {
    if (bounds.contains(mousePos)) {
        dragging = true;
        handleDrag(mousePos);
        return true;
    }
    return false;
}

void Slider::handleDrag(sf::Vector2f mousePos) {
    if (!value) return;
    
    float normalized = (mousePos.x - bounds.position.x) / bounds.size.x;
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    if (isLogarithmic) {
        // Logarithmic scale: map 0-1 to log(min)-log(max)
        float logMin = std::log(minValue);
        float logMax = std::log(maxValue);
        float logValue = logMin + normalized * (logMax - logMin);
        *value = std::exp(logValue);
    } else {
        *value = minValue + normalized * (maxValue - minValue);
    }
    
    if (isInteger) {
        *value = std::round(*value);
    }
}

void Slider::draw(sf::RenderWindow& window, const sf::Font& font) {
    // Track background
    sf::RectangleShape track({bounds.size.x, 6.0f});
    track.setPosition({bounds.position.x, bounds.position.y + bounds.size.y - 10.0f});
    track.setFillColor(sf::Color(40, 40, 45));
    track.setOutlineColor(sf::Color(60, 60, 65));
    track.setOutlineThickness(1);
    window.draw(track);
    
    // Fill - calculate normalized position
    float normalized;
    if (isLogarithmic) {
        float logMin = std::log(minValue);
        float logMax = std::log(maxValue);
        float logVal = std::log(*value);
        normalized = (logVal - logMin) / (logMax - logMin);
    } else {
        normalized = (*value - minValue) / (maxValue - minValue);
    }
    normalized = std::max(0.0f, std::min(1.0f, normalized));
    
    sf::RectangleShape fill({bounds.size.x * normalized, 6.0f});
    fill.setPosition({bounds.position.x, bounds.position.y + bounds.size.y - 10.0f});
    fill.setFillColor(sf::Color(70, 130, 180));
    window.draw(fill);
    
    // Handle
    float handleX = bounds.position.x + normalized * bounds.size.x;
    float handleY = bounds.position.y + bounds.size.y - 7.0f;
    
    sf::CircleShape handle(7.0f);
    handle.setOrigin({7.0f, 7.0f});
    handle.setPosition({handleX, handleY});
    handle.setFillColor(dragging ? sf::Color(100, 160, 220) : sf::Color(85, 145, 200));
    handle.setOutlineColor(sf::Color(255, 255, 255, 180));
    handle.setOutlineThickness(2);
    window.draw(handle);
    
    // Label - increased size
    sf::Text labelText(font, label, 14);
    labelText.setPosition({bounds.position.x, bounds.position.y});
    labelText.setFillColor(sf::Color(200, 200, 200));
    window.draw(labelText);
    
    // Value - increased size
    std::ostringstream ss;
    if (isInteger) {
        ss << static_cast<int>(*value);
    } else {
        ss << std::fixed << std::setprecision(2) << *value;
    }
    
    sf::Text valueText(font, ss.str(), 14);
    sf::FloatRect valueBounds = valueText.getLocalBounds();
    valueText.setPosition({bounds.position.x + bounds.size.x - valueBounds.size.x, bounds.position.y});
    valueText.setFillColor(sf::Color::White);
    window.draw(valueText);
}

Toggle::Toggle() : value(nullptr) {}

Toggle::Toggle(float x, float y, float width, float height,
               bool* val, const std::string& lbl)
    : bounds({x, y}, {width, height}), value(val), label(lbl) {}

bool Toggle::handleClick(sf::Vector2f mousePos) {
    if (bounds.contains(mousePos) && value) {
        *value = !(*value);
        return true;
    }
    return false;
}

void Toggle::draw(sf::RenderWindow& window, const sf::Font& font) {
    bool isOn = value && *value;
    
    float switchWidth = 36.0f;
    float switchHeight = 18.0f;
    float switchX = bounds.position.x + bounds.size.x - switchWidth - 5;
    float switchY = bounds.position.y + (bounds.size.y - switchHeight) / 2;
    
    sf::RectangleShape switchBg({switchWidth, switchHeight});
    switchBg.setPosition({switchX, switchY});
    switchBg.setFillColor(isOn ? sf::Color(70, 150, 70) : sf::Color(60, 60, 65));
    switchBg.setOutlineColor(sf::Color(80, 80, 85));
    switchBg.setOutlineThickness(1);
    window.draw(switchBg);
    
    float knobRadius = 7.0f;
    float knobX = isOn ? (switchX + switchWidth - knobRadius - 3) : (switchX + knobRadius + 3);
    float knobY = switchY + switchHeight / 2;
    
    sf::CircleShape knob(knobRadius);
    knob.setOrigin({knobRadius, knobRadius});
    knob.setPosition({knobX, knobY});
    knob.setFillColor(sf::Color::White);
    window.draw(knob);
    
    sf::Text text(font, label, 14);
    text.setPosition({bounds.position.x, bounds.position.y + (bounds.size.y - 14) / 2});
    text.setFillColor(sf::Color(200, 200, 200));
    window.draw(text);
}

Button::Button() {}

Button::Button(float x, float y, float width, float height, const std::string& lbl)
    : bounds({x, y}, {width, height}), label(lbl) {}

bool Button::handleClick(sf::Vector2f mousePos) {
    return bounds.contains(mousePos);
}

void Button::draw(sf::RenderWindow& window, const sf::Font& font, bool highlighted) {
    sf::RectangleShape bg(bounds.size);
    bg.setPosition(bounds.position);
    bg.setFillColor(highlighted ? sf::Color(80, 140, 180) : sf::Color(50, 52, 58));
    bg.setOutlineColor(sf::Color(70, 72, 78));
    bg.setOutlineThickness(1);
    window.draw(bg);
    
    sf::Text text(font, label, 12);
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition({
        bounds.position.x + (bounds.size.x - textBounds.size.x) / 2,
        bounds.position.y + (bounds.size.y - textBounds.size.y) / 2 - 2
    });
    text.setFillColor(sf::Color(220, 220, 225));
    window.draw(text);
}

// ============================================================================
// RENDERER IMPLEMENTATION
// ============================================================================

Renderer::Renderer()
    : fontLoaded(false)
    , uiPanelWidth(250.0f)
    , bottomBarHeight(80.0f)
    , worldDisplaySize(600.0f)
    , worldScale(1.0f)
    , uiInitialized(false)
    , controlsBottomY(0.0f)
    , isDraggingSlider(false)
    , activeSliderIndex(-1)
    , showDNAPanel(false)
    , windowWidth(1600)
    , windowHeight(1000)
    , editingObstacles(false)
    , editingSurvivalZones(false)
    , isDrawing(false)
    , isErasing(false)
    , drawStart(0.0f, 0.0f) {}

void Renderer::initialize(int winWidth, int winHeight) {
    windowWidth = winWidth;
    windowHeight = winHeight;
    recalculateLayout();
}

void Renderer::recalculateLayout() {
    // Responsive sidebar width based on window size
    uiPanelWidth = std::max(220.0f, std::min(280.0f, windowWidth * 0.18f));
    
    // Bottom bar - fixed height that fits content
    bottomBarHeight = 68.0f;
    
    // World display takes remaining space with proper margins
    float margin = 8.0f;
    float availableWidth = windowWidth - uiPanelWidth - margin * 2;
    float availableHeight = windowHeight - bottomBarHeight - margin;
    worldDisplaySize = std::min(availableWidth, availableHeight);
    
    // Position world centered in available space
    worldOffset.x = uiPanelWidth + (availableWidth - worldDisplaySize) / 2 + margin;
    worldOffset.y = (availableHeight - worldDisplaySize) / 2;
}

bool Renderer::loadFont(const std::string& fontPath) {
    fontLoaded = font.openFromFile(fontPath);
    return fontLoaded;
}

void Renderer::setupUI(Simulation& sim) {
    sliders.clear();
    toggles.clear();
    buttons.clear();
    
    float padding = 10.0f;
    float x = padding;
    float w = uiPanelWidth - padding * 2;
    float sliderH = 32.0f;
    float toggleH = 26.0f;
    float buttonH = 28.0f;
    float sectionGap = 14.0f;
    float itemGap = 4.0f;
    
    // Start after header area
    float y = 52.0f;
    
    // === CONTROL BUTTONS ===
    float btnGap = 6.0f;
    float btnWidth = (w - btnGap * 2) / 3;
    buttons.push_back(Button(x, y, btnWidth, buttonH, "Pause"));
    buttons.push_back(Button(x + btnWidth + btnGap, y, btnWidth, buttonH, "Reset"));
    buttons.push_back(Button(x + (btnWidth + btnGap) * 2, y, btnWidth, buttonH, "Defaults"));
    y += buttonH + sectionGap;
    
    // === SIMULATION SLIDERS ===
    sliders.push_back(Slider(x, y, w, sliderH, 1.0f, 100000.0f, 
        &sim.speedMultiplier, "Speed", true, true));
    y += sliderH + itemGap;
    
    sliders.push_back(Slider(x, y, w, sliderH, 5.0f, 60.0f, 
        &sim.config.generationTime, "Gen Time (sec)"));
    y += sliderH + itemGap;
    
    sliders.push_back(Slider(x, y, w, sliderH, 20.0f, 500.0f, 
        &sim.config.initialPopulation, "Population", true));
    y += sliderH + itemGap;
    
    sliders.push_back(Slider(x, y, w, sliderH, 50.0f, 500.0f, 
        &sim.config.peepSpeed, "Peep Speed", true));
    y += sliderH + sectionGap;
    
    // === BRAIN & GENETICS ===
    sliders.push_back(Slider(x, y, w, sliderH, 1.0f, 128.0f, 
        &sim.config.genesPerPeep, "Genes", true));
    y += sliderH + itemGap;
    
    sliders.push_back(Slider(x, y, w, sliderH, 1.0f, 32.0f, 
        &sim.config.hiddenNeurons, "Hidden Neurons", true));
    y += sliderH + itemGap;
    
    sliders.push_back(Slider(x, y, w, sliderH, 0.0f, 0.5f, 
        &sim.config.mutationRate, "Mutation Rate"));
    y += sliderH + itemGap;
    
    toggles.push_back(Toggle(x, y, w, toggleH, &sim.config.useCrossover, "Sexual Reproduction"));
    y += toggleH + sectionGap;
    
    // === HUNGER SYSTEM ===
    toggles.push_back(Toggle(x, y, w, toggleH, &sim.config.hungerEnabled, "Hunger System"));
    y += toggleH + itemGap;
    
    sliders.push_back(Slider(x, y, w, sliderH, 20.0f, 200.0f, 
        &sim.config.foodCount, "Food Count", true));
    y += sliderH + sectionGap;
    
    // === ENVIRONMENT ===
    toggles.push_back(Toggle(x, y, w, toggleH, &sim.config.obstaclesEnabled, "Obstacles"));
    y += toggleH + itemGap;
    
    buttons.push_back(Button(x, y, w, buttonH, "Edit Obstacles"));
    y += buttonH + itemGap;
    
    toggles.push_back(Toggle(x, y, w, toggleH, &sim.config.survivalZoneEnabled, "Survival Zones"));
    y += toggleH + itemGap;
    
    buttons.push_back(Button(x, y, w, buttonH, "Edit Zones"));
    y += buttonH;
    
    controlsBottomY = y;
    uiInitialized = true;
}

void Renderer::render(sf::RenderWindow& window, Simulation& sim) {
    // Check for window resize
    sf::Vector2u winSize = window.getSize();
    if (static_cast<int>(winSize.x) != windowWidth || static_cast<int>(winSize.y) != windowHeight) {
        windowWidth = static_cast<int>(winSize.x);
        windowHeight = static_cast<int>(winSize.y);
        recalculateLayout();
        uiInitialized = false;  // Force UI rebuild with new dimensions
    }
    
    if (!uiInitialized) {
        setupUI(sim);
    }
    
    window.clear(sf::Color(25, 25, 30));
    
    drawWorld(window, sim);
    drawUI(window, sim);
    
    // Draw edit mode overlay if editing
    if (editingObstacles || editingSurvivalZones) {
        drawEditModeOverlay(window, sim);
    }
    
    // Draw DNA panel if a peep is selected
    if (sim.selectedPeep >= 0 && sim.selectedPeep < static_cast<int>(sim.peeps.size())) {
        drawDNAPanel(window, sim);
    }
    
    window.display();
}

void Renderer::drawWorld(sf::RenderWindow& window, Simulation& sim) {
    worldScale = worldDisplaySize / sim.config.worldSize;
    
    // World glow
    sf::RectangleShape worldGlow({worldDisplaySize + 8, worldDisplaySize + 8});
    worldGlow.setPosition({worldOffset.x - 4, worldOffset.y - 4});
    worldGlow.setFillColor(sf::Color(40, 45, 55));
    worldGlow.setOutlineColor(sf::Color(60, 65, 75));
    worldGlow.setOutlineThickness(2);
    window.draw(worldGlow);
    
    // Main world area
    sf::RectangleShape worldBg({worldDisplaySize, worldDisplaySize});
    worldBg.setPosition(worldOffset);
    worldBg.setFillColor(sf::Color(20, 22, 28));
    window.draw(worldBg);
    
    // Grid
    drawGrid(window, sim);
    
    // Survival zones (draw first so they're behind everything)
    if (sim.config.survivalZoneEnabled) {
        drawSurvivalZones(window, sim);
    }
    
    // Obstacles
    if (sim.config.obstaclesEnabled) {
        drawObstacles(window, sim);
    }
    
    // Food
    if (sim.config.hungerEnabled) {
        drawFood(window, sim);
    }
    
    // Peeps
    drawPeeps(window, sim);
    
    // Border
    sf::RectangleShape border({worldDisplaySize, worldDisplaySize});
    border.setPosition(worldOffset);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(80, 85, 95));
    border.setOutlineThickness(2);
    window.draw(border);
}

void Renderer::drawGrid(sf::RenderWindow& window, Simulation& sim) {
    float gridSize = 50.0f * worldScale;
    sf::Color gridColor(35, 38, 48);
    
    for (float x = gridSize; x < worldDisplaySize; x += gridSize) {
        sf::RectangleShape line({1.0f, worldDisplaySize});
        line.setPosition({worldOffset.x + x, worldOffset.y});
        line.setFillColor(gridColor);
        window.draw(line);
    }
    
    for (float y = gridSize; y < worldDisplaySize; y += gridSize) {
        sf::RectangleShape line({worldDisplaySize, 1.0f});
        line.setPosition({worldOffset.x, worldOffset.y + y});
        line.setFillColor(gridColor);
        window.draw(line);
    }
}

void Renderer::drawPeeps(sf::RenderWindow& window, Simulation& sim) {
    float radius = sim.config.peepSize * worldScale;
    if (radius < 2.0f) radius = 2.0f;
    
    sf::CircleShape dot(radius);
    dot.setOrigin({radius, radius});
    
    for (size_t i = 0; i < sim.peeps.size(); i++) {
        const Peep& p = sim.peeps[i];
        if (!p.alive) continue;
        
        sf::Vector2f screenPos = worldOffset + p.position * worldScale;
        dot.setPosition(screenPos);
        
        sf::Color peepColor;
        if (sim.config.showHungerColors && sim.config.hungerEnabled) {
            float hungerRatio = std::max(0.0f, std::min(1.0f, p.hunger / sim.config.maxHunger));
            uint8_t r = static_cast<uint8_t>((1.0f - hungerRatio) * 220 + 35);
            uint8_t g = static_cast<uint8_t>(hungerRatio * 200 + 55);
            peepColor = sf::Color(r, g, 80);
        } else {
            peepColor = sf::Color(p.colorR, p.colorG, p.colorB);
        }
        
        dot.setFillColor(peepColor);
        
        if (static_cast<int>(i) == sim.selectedPeep) {
            dot.setOutlineColor(sf::Color(255, 220, 100));
            dot.setOutlineThickness(3);
        } else {
            dot.setOutlineThickness(0);
        }
        
        window.draw(dot);
        
        if (sim.config.showSenseRange) {
            float rangeRadius = sim.config.peepSenseRange * worldScale;
            sf::CircleShape range(rangeRadius);
            range.setOrigin({rangeRadius, rangeRadius});
            range.setPosition(screenPos);
            range.setFillColor(sf::Color(peepColor.r, peepColor.g, peepColor.b, 20));
            range.setOutlineColor(sf::Color(peepColor.r, peepColor.g, peepColor.b, 60));
            range.setOutlineThickness(1);
            window.draw(range);
        }
    }
}

void Renderer::drawFood(sf::RenderWindow& window, Simulation& sim) {
    float radius = sim.config.foodSize * worldScale;
    if (radius < 2.0f) radius = 2.0f;
    
    sf::CircleShape foodDot(radius);
    foodDot.setOrigin({radius, radius});
    
    for (const Food& f : sim.foods) {
        if (f.eaten) continue;
        
        sf::Vector2f screenPos = worldOffset + f.position * worldScale;
        foodDot.setPosition(screenPos);
        foodDot.setFillColor(sf::Color(80, 180, 100));
        foodDot.setOutlineColor(sf::Color(100, 220, 120, 100));
        foodDot.setOutlineThickness(2);
        window.draw(foodDot);
    }
}

void Renderer::drawObstacles(sf::RenderWindow& window, Simulation& sim) {
    for (const Obstacle& obs : sim.obstacles) {
        sf::RectangleShape rect(obs.size * worldScale);
        rect.setPosition(worldOffset + obs.position * worldScale);
        rect.setFillColor(sf::Color(50, 52, 60));
        rect.setOutlineColor(sf::Color(70, 72, 80));
        rect.setOutlineThickness(1);
        window.draw(rect);
    }
}

void Renderer::drawSurvivalZones(sf::RenderWindow& window, Simulation& sim) {
    // Draw user-defined survival zones
    for (const SurvivalZone& zone : sim.survivalZones) {
        sf::RectangleShape rect(zone.size * worldScale);
        rect.setPosition(worldOffset + zone.position * worldScale);
        rect.setFillColor(sf::Color(80, 180, 100, 50));
        rect.setOutlineColor(sf::Color(100, 220, 120, 150));
        rect.setOutlineThickness(2);
        window.draw(rect);
    }
    
    // If no zones defined but mode enabled, show fallback preset zone
    if (sim.survivalZones.empty()) {
        float half = sim.config.worldSize / 2.0f;
        int mode = static_cast<int>(sim.config.survivalMode);
        
        sf::RectangleShape zone;
        zone.setFillColor(sf::Color(80, 180, 100, 40));
        zone.setOutlineColor(sf::Color(80, 180, 100, 80));
        zone.setOutlineThickness(1);
        
        switch (mode) {
            case 0:
                zone.setSize({half * worldScale, sim.config.worldSize * worldScale});
                zone.setPosition(worldOffset + sf::Vector2f(half * worldScale, 0));
                window.draw(zone);
                break;
            case 1:
                zone.setSize({half * worldScale, sim.config.worldSize * worldScale});
                zone.setPosition(worldOffset);
                window.draw(zone);
                break;
            default:
                break;
        }
    }
}

void Renderer::drawUI(sf::RenderWindow& window, Simulation& sim) {
    // === SIDEBAR ===
    sf::RectangleShape sidebar({uiPanelWidth, static_cast<float>(windowHeight)});
    sidebar.setFillColor(sf::Color(32, 34, 40));
    window.draw(sidebar);
    
    // Sidebar border
    sf::RectangleShape sidebarBorder({2, static_cast<float>(windowHeight)});
    sidebarBorder.setPosition({uiPanelWidth - 2, 0});
    sidebarBorder.setFillColor(sf::Color(50, 52, 60));
    window.draw(sidebarBorder);
    
    // === BOTTOM BAR ===
    float bottomBarY = static_cast<float>(windowHeight) - bottomBarHeight;
    sf::RectangleShape bottomBar({static_cast<float>(windowWidth) - uiPanelWidth, bottomBarHeight});
    bottomBar.setPosition({uiPanelWidth, bottomBarY});
    bottomBar.setFillColor(sf::Color(32, 34, 40));
    window.draw(bottomBar);
    
    // Bottom bar top border
    sf::RectangleShape bottomBarBorder({static_cast<float>(windowWidth) - uiPanelWidth, 2});
    bottomBarBorder.setPosition({uiPanelWidth, bottomBarY});
    bottomBarBorder.setFillColor(sf::Color(50, 52, 60));
    window.draw(bottomBarBorder);
    
    if (!fontLoaded) return;
    
    drawStats(window, sim);
    drawControls(window, sim);
    drawBottomBar(window, sim);
}

void Renderer::drawStats(sf::RenderWindow& window, Simulation& sim) {
    float x = 12.0f;
    float y = 8.0f;
    
    // Title + status on same line
    sf::Text title(font, "Evolution Sim", 16);
    title.setPosition({x, y});
    title.setFillColor(sf::Color(220, 220, 225));
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    
    // Status indicator
    float timeLeft = sim.config.generationTime - sim.generationTimer;
    if (timeLeft < 0) timeLeft = 0;
    
    std::ostringstream ss;
    if (sim.paused) {
        sf::Text pauseText(font, "PAUSED", 11);
        pauseText.setPosition({x + 120.0f, y + 2.0f});
        pauseText.setFillColor(sf::Color(255, 180, 80));
        window.draw(pauseText);
    } else {
        ss << std::fixed << std::setprecision(1) << timeLeft << "s";
        sf::Text timeText(font, ss.str(), 11);
        timeText.setPosition({x + 120.0f, y + 2.0f});
        timeText.setFillColor(sf::Color(140, 140, 145));
        window.draw(timeText);
    }
    
    // Separator line
    sf::RectangleShape sep({uiPanelWidth - 20.0f, 1.0f});
    sep.setPosition({x, y + 24.0f});
    sep.setFillColor(sf::Color(50, 52, 60));
    window.draw(sep);
}

void Renderer::drawControls(sf::RenderWindow& window, Simulation& sim) {
    if (!buttons.empty()) {
        buttons[0].label = sim.paused ? "Resume" : "Pause";
    }
    if (buttons.size() > 3) {
        buttons[3].label = editingObstacles ? "Done Editing" : "Edit Obstacles";
    }
    if (buttons.size() > 4) {
        buttons[4].label = editingSurvivalZones ? "Done Editing" : "Edit Zones";
    }
    
    for (size_t i = 0; i < buttons.size(); i++) {
        bool highlighted = (i == 3 && editingObstacles) || (i == 4 && editingSurvivalZones);
        buttons[i].draw(window, font, highlighted);
    }
    
    for (Slider& slider : sliders) {
        slider.draw(window, font);
    }
    
    for (Toggle& toggle : toggles) {
        toggle.draw(window, font);
    }
}

void Renderer::drawBottomBar(sf::RenderWindow& window, Simulation& sim) {
    if (!fontLoaded) return;
    
    float bottomBarY = static_cast<float>(windowHeight) - bottomBarHeight;
    float barWidth = static_cast<float>(windowWidth) - uiPanelWidth;
    
    // Content area with padding from edges
    float padX = 12.0f;
    float padY = 8.0f;
    float contentY = bottomBarY + padY;
    float contentH = bottomBarHeight - padY * 2;
    
    // === DISPLAY TOGGLES - right side with more padding ===
    float toggleW = 145.0f;
    float toggleH = 24.0f;
    float toggleGap = 4.0f;
    float toggleAreaX = static_cast<float>(windowWidth) - toggleW - padX * 2;
    float toggleY = contentY + 2.0f;
    
    // Sense Range toggle
    {
        bool isOn = sim.config.showSenseRange;
        float switchW = 32.0f, switchH = 16.0f;
        float switchX = toggleAreaX + toggleW - switchW;
        float switchY = toggleY + (toggleH - switchH) / 2;
        
        bottomBarToggles[0] = sf::FloatRect({toggleAreaX, toggleY}, {toggleW, toggleH});
        
        sf::RectangleShape bg({switchW, switchH});
        bg.setPosition({switchX, switchY});
        bg.setFillColor(isOn ? sf::Color(70, 150, 70) : sf::Color(50, 50, 55));
        bg.setOutlineColor(sf::Color(60, 60, 65));
        bg.setOutlineThickness(1);
        window.draw(bg);
        
        float kr = 6.0f;
        sf::CircleShape knob(kr);
        knob.setOrigin({kr, kr});
        knob.setPosition({isOn ? switchX + switchW - kr - 2 : switchX + kr + 2, switchY + switchH / 2});
        knob.setFillColor(sf::Color::White);
        window.draw(knob);
        
        sf::Text lbl(font, "Sense Range", 13);
        lbl.setPosition({toggleAreaX, toggleY + (toggleH - 13) / 2});
        lbl.setFillColor(sf::Color(200, 200, 205));
        window.draw(lbl);
    }
    toggleY += toggleH + toggleGap;
    
    // Hunger Colors toggle
    {
        bool isOn = sim.config.showHungerColors;
        float switchW = 32.0f, switchH = 16.0f;
        float switchX = toggleAreaX + toggleW - switchW;
        float switchY = toggleY + (toggleH - switchH) / 2;
        
        bottomBarToggles[1] = sf::FloatRect({toggleAreaX, toggleY}, {toggleW, toggleH});
        
        sf::RectangleShape bg({switchW, switchH});
        bg.setPosition({switchX, switchY});
        bg.setFillColor(isOn ? sf::Color(70, 150, 70) : sf::Color(50, 50, 55));
        bg.setOutlineColor(sf::Color(60, 60, 65));
        bg.setOutlineThickness(1);
        window.draw(bg);
        
        float kr = 6.0f;
        sf::CircleShape knob(kr);
        knob.setOrigin({kr, kr});
        knob.setPosition({isOn ? switchX + switchW - kr - 2 : switchX + kr + 2, switchY + switchH / 2});
        knob.setFillColor(sf::Color::White);
        window.draw(knob);
        
        sf::Text lbl(font, "Hunger Colors", 13);
        lbl.setPosition({toggleAreaX, toggleY + (toggleH - 13) / 2});
        lbl.setFillColor(sf::Color(200, 200, 205));
        window.draw(lbl);
    }
    
    // === STATS SECTION ===
    float statsWidth = 110.0f;
    float statsX = toggleAreaX - statsWidth - padX;
    float lineH = 16.0f;
    float statsY = contentY + 4.0f;
    
    std::ostringstream ss;
    ss << "Gen " << sim.currentGeneration;
    sf::Text genText(font, ss.str(), 13);
    genText.setPosition({statsX, statsY});
    genText.setFillColor(sf::Color(200, 200, 205));
    window.draw(genText);
    
    ss.str("");
    ss << sim.stats.aliveCount << "/" << sim.peeps.size();
    sf::Text popText(font, ss.str(), 12);
    popText.setPosition({statsX, statsY + lineH + 2.0f});
    popText.setFillColor(sf::Color(160, 160, 165));
    window.draw(popText);
    
    // Survival rate
    if (!sim.stats.survivalHistory.empty()) {
        float lastSurvival = sim.stats.survivalHistory.back();
        ss.str("");
        ss << std::fixed << std::setprecision(0) << (lastSurvival * 100.0f) << "%";
        sf::Text survText(font, ss.str(), 12);
        survText.setPosition({statsX + 55.0f, statsY + lineH + 2.0f});
        
        sf::Color survColor = lastSurvival >= 0.5f ? sf::Color(100, 200, 120) :
                              lastSurvival >= 0.2f ? sf::Color(200, 200, 100) :
                              sf::Color(200, 120, 100);
        survText.setFillColor(survColor);
        window.draw(survText);
    }
    
    // === GRAPH - takes remaining space dynamically ===
    float graphX = uiPanelWidth + padX;
    float graphWidth = statsX - graphX - padX;
    float graphHeight = contentH;
    
    drawGraph(window, sim.stats.survivalHistory,
              sf::FloatRect({graphX, contentY}, {graphWidth, graphHeight}),
              sf::Color(100, 200, 150), "Survival Rate");
}

void Renderer::drawDNAPanel(sf::RenderWindow& window, Simulation& sim) {
    if (!fontLoaded) return;
    if (sim.selectedPeep < 0 || sim.selectedPeep >= static_cast<int>(sim.peeps.size())) return;
    
    const Peep& p = sim.peeps[sim.selectedPeep];
    if (!p.alive) return;
    
    // Panel dimensions
    float panelWidth = 380.0f;
    float lineHeight = 16.0f;
    float panelHeight = 90.0f + p.genome.size() * lineHeight * 2 + 20.0f;
    
    // Position at bottom-right of world
    float px = worldOffset.x + worldDisplaySize - panelWidth - 10;
    float py = worldOffset.y + worldDisplaySize - panelHeight - 10;
    
    // Clamp to screen
    if (py < worldOffset.y + 10) py = worldOffset.y + 10;
    
    // Background
    sf::RectangleShape bg({panelWidth, panelHeight});
    bg.setPosition({px, py});
    bg.setFillColor(sf::Color(15, 17, 22, 240));
    bg.setOutlineColor(sf::Color(60, 65, 75));
    bg.setOutlineThickness(1);
    window.draw(bg);
    
    float textY = py + 10;
    
    // Title
    sf::Text title(font, "Selected Peep - DNA View", 15);
    title.setPosition({px + 10, textY});
    title.setFillColor(sf::Color(255, 220, 100));
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    textY += 24;
    
    // Stats line
    std::ostringstream ss;
    ss << "Hunger: " << std::fixed << std::setprecision(0) << p.hunger
       << "  |  Age: " << std::setprecision(1) << p.age << "s"
       << "  |  Gen: " << p.generation;
    
    sf::Text statsText(font, ss.str(), 12);
    statsText.setPosition({px + 10, textY});
    statsText.setFillColor(sf::Color(160, 160, 165));
    window.draw(statsText);
    textY += 22;
    
    // Separator
    sf::RectangleShape sep({panelWidth - 20, 1});
    sep.setPosition({px + 10, textY});
    sep.setFillColor(sf::Color(50, 52, 60));
    window.draw(sep);
    textY += 8;
    
    // DNA header
    sf::Text dnaHeader(font, "GENOME (32-bit genes):", 12);
    dnaHeader.setPosition({px + 10, textY});
    dnaHeader.setFillColor(sf::Color(140, 140, 145));
    window.draw(dnaHeader);
    
    // Mutation legend if peep has mutations
    bool hasMutations = false;
    for (size_t i = 0; i < p.mutatedBits.size() && i < p.genome.size(); i++) {
        if (p.mutatedBits[i] != 0) { hasMutations = true; break; }
    }
    if (hasMutations) {
        sf::Text legend(font, " (mutated bits in red)", 11);
        legend.setPosition({px + 175, textY + 1});
        legend.setFillColor(sf::Color(255, 100, 100));
        window.draw(legend);
    }
    textY += 18;
    
    // Each gene
    for (size_t i = 0; i < p.genome.size(); i++) {
        uint32_t gene = p.genome[i];
        uint32_t mutMask = (i < p.mutatedBits.size()) ? p.mutatedBits[i] : 0;
        
        // Binary representation
        std::string binary = Gene::toBinaryString(gene);
        
        // Gene number
        ss.str("");
        ss << "G" << i << ": ";
        sf::Text geneNum(font, ss.str(), 11);
        geneNum.setPosition({px + 10, textY});
        geneNum.setFillColor(sf::Color(120, 120, 125));
        window.draw(geneNum);
        
        // Render bits individually to allow mutation highlighting
        // Format: "XXXX XXXX XXXXXXXXXXXXXXXXXXXXXXXX" (4 + space + 4 + space + 24)
        // Binary string has spaces at positions 4 and 9
        float bx = px + 40;
        float charWidth = 7.0f;  // Approximate width of a character at size 11
        
        // Map binary string position to actual bit position (31 to 0, left to right)
        // String: pos 0-3 = bits 31-28, pos 5-8 = bits 27-24, pos 10-33 = bits 23-0
        for (size_t strPos = 0; strPos < binary.size(); strPos++) {
            char ch = binary[strPos];
            
            // Handle spaces
            if (ch == ' ') {
                bx += charWidth * 0.6f;
                continue;
            }
            
            // Determine actual bit index (31 to 0)
            int bitIndex;
            if (strPos < 4) {
                bitIndex = 31 - static_cast<int>(strPos);
            } else if (strPos < 9) {
                bitIndex = 31 - static_cast<int>(strPos - 1); // account for space at pos 4
            } else {
                bitIndex = 31 - static_cast<int>(strPos - 2); // account for spaces at pos 4 and 9
            }
            
            // Check if this bit was mutated
            bool isMutated = (mutMask & (1u << bitIndex)) != 0;
            
            // Determine base color based on section
            sf::Color baseColor;
            if (bitIndex >= 28) {
                baseColor = sf::Color(100, 200, 220);  // Source bits (cyan)
            } else if (bitIndex >= 24) {
                baseColor = sf::Color(100, 220, 120);  // Dest bits (green)
            } else {
                baseColor = sf::Color(140, 140, 145);  // Weight bits (gray)
            }
            
            // Override with red/orange if mutated
            sf::Color finalColor = isMutated ? sf::Color(255, 80, 60) : baseColor;
            
            std::string charStr(1, ch);
            sf::Text bitText(font, charStr, 11);
            bitText.setPosition({bx, textY});
            bitText.setFillColor(finalColor);
            if (isMutated) {
                bitText.setStyle(sf::Text::Bold);
            }
            window.draw(bitText);
            
            bx += charWidth;
        }
        
        textY += lineHeight;
        
        // Human readable
        std::string readable = Gene::toDisplayString(gene);
        sf::Text readableText(font, readable, 11);
        readableText.setPosition({px + 40, textY});
        readableText.setFillColor(sf::Color(180, 180, 185));
        window.draw(readableText);
        
        textY += lineHeight + 2;
    }
}

void Renderer::drawEditModeOverlay(sf::RenderWindow& window, Simulation& sim) {
    if (!fontLoaded) return;
    
    // Determine what we're editing
    bool isObstacleMode = editingObstacles;
    bool isZoneMode = editingSurvivalZones;
    
    // Colors based on mode
    sf::Color fillColor = isObstacleMode ? sf::Color(80, 60, 60, 180) : sf::Color(60, 100, 60, 180);
    sf::Color outlineColor = isObstacleMode ? sf::Color(200, 100, 100) : sf::Color(100, 220, 120);
    sf::Color previewFill = isObstacleMode ? sf::Color(200, 100, 100, 100) : sf::Color(100, 220, 120, 100);
    sf::Color previewOutline = isObstacleMode ? sf::Color(255, 150, 150) : sf::Color(150, 255, 170);
    sf::Color borderColor = isObstacleMode ? sf::Color(255, 180, 80) : sf::Color(100, 255, 150);
    
    // Dim the world slightly to indicate edit mode
    sf::RectangleShape dimOverlay({worldDisplaySize, worldDisplaySize});
    dimOverlay.setPosition(worldOffset);
    dimOverlay.setFillColor(sf::Color(0, 0, 0, 40));
    window.draw(dimOverlay);
    
    // Highlight existing items
    if (isObstacleMode) {
        for (const Obstacle& obs : sim.obstacles) {
            sf::RectangleShape rect(obs.size * worldScale);
            rect.setPosition(worldOffset + obs.position * worldScale);
            rect.setFillColor(fillColor);
            rect.setOutlineColor(outlineColor);
            rect.setOutlineThickness(2);
            window.draw(rect);
        }
    } else if (isZoneMode) {
        for (const SurvivalZone& zone : sim.survivalZones) {
            sf::RectangleShape rect(zone.size * worldScale);
            rect.setPosition(worldOffset + zone.position * worldScale);
            rect.setFillColor(fillColor);
            rect.setOutlineColor(outlineColor);
            rect.setOutlineThickness(2);
            window.draw(rect);
        }
    }
    
    // Draw preview of shape being created
    if (isDrawing && !isErasing) {
        sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos(static_cast<float>(mousePixel.x), static_cast<float>(mousePixel.y));
        
        // Convert to world coords
        sf::Vector2f worldEnd = (mousePos - worldOffset) / worldScale;
        
        // Calculate rectangle from start to current
        float x = std::min(drawStart.x, worldEnd.x);
        float y = std::min(drawStart.y, worldEnd.y);
        float w = std::abs(worldEnd.x - drawStart.x);
        float h = std::abs(worldEnd.y - drawStart.y);
        
        if (w > 5 && h > 5) {
            sf::RectangleShape preview({w * worldScale, h * worldScale});
            preview.setPosition(worldOffset + sf::Vector2f(x, y) * worldScale);
            preview.setFillColor(previewFill);
            preview.setOutlineColor(previewOutline);
            preview.setOutlineThickness(2);
            window.draw(preview);
        }
    }
    
    // Edit mode border
    sf::RectangleShape editBorder({worldDisplaySize, worldDisplaySize});
    editBorder.setPosition(worldOffset);
    editBorder.setFillColor(sf::Color::Transparent);
    editBorder.setOutlineColor(borderColor);
    editBorder.setOutlineThickness(3);
    window.draw(editBorder);
    
    // Instructions at top
    std::string modeText = isObstacleMode ? "OBSTACLES" : "SURVIVAL ZONES";
    std::string instrText = "EDITING " + modeText + ": Left-drag to draw | Right-click to erase | ESC to exit";
    sf::Text instructions(font, instrText, 14);
    sf::FloatRect textBounds = instructions.getLocalBounds();
    instructions.setPosition({worldOffset.x + (worldDisplaySize - textBounds.size.x) / 2, worldOffset.y + 10});
    instructions.setFillColor(sf::Color(255, 220, 100));
    window.draw(instructions);
}

void Renderer::drawGraph(sf::RenderWindow& window, const std::vector<float>& data,
                         sf::FloatRect bounds, sf::Color color, const std::string& label) {
    if (data.empty()) return;
    
    // Left margin for Y-axis labels
    float leftMargin = 30.0f;
    float graphX = bounds.position.x + leftMargin;
    float graphWidth = bounds.size.x - leftMargin;
    
    sf::RectangleShape bg(bounds.size);
    bg.setPosition(bounds.position);
    bg.setFillColor(sf::Color(25, 27, 32));
    bg.setOutlineColor(sf::Color(50, 52, 58));
    bg.setOutlineThickness(1);
    window.draw(bg);
    
    // Title label
    sf::Text text(font, label, 10);
    text.setPosition({graphX + 5, bounds.position.y + 2});
    text.setFillColor(sf::Color(120, 120, 125));
    window.draw(text);
    
    // Y-axis labels (0%, 50%, 100%)
    float graphTop = bounds.position.y + 15;
    float graphBottom = bounds.position.y + bounds.size.y;
    float graphHeight = graphBottom - graphTop;
    
    sf::Text yLabel100(font, "100%", 9);
    yLabel100.setPosition({bounds.position.x + 2, graphTop - 3});
    yLabel100.setFillColor(sf::Color(90, 90, 95));
    window.draw(yLabel100);
    
    sf::Text yLabel50(font, "50%", 9);
    yLabel50.setPosition({bounds.position.x + 5, graphTop + graphHeight / 2 - 5});
    yLabel50.setFillColor(sf::Color(90, 90, 95));
    window.draw(yLabel50);
    
    sf::Text yLabel0(font, "0%", 9);
    yLabel0.setPosition({bounds.position.x + 8, graphBottom - 12});
    yLabel0.setFillColor(sf::Color(90, 90, 95));
    window.draw(yLabel0);
    
    // Draw horizontal grid lines at 50%
    sf::RectangleShape gridLine({graphWidth, 1});
    gridLine.setPosition({graphX, graphTop + graphHeight / 2});
    gridLine.setFillColor(sf::Color(40, 42, 48));
    window.draw(gridLine);
    
    if (data.size() < 2) return;
    
    // For survival rate, maxVal is always 1.0 (100%)
    float maxVal = 1.0f;
    
    float xStep = graphWidth / static_cast<float>(data.size() - 1);
    
    sf::VertexArray lines(sf::PrimitiveType::LineStrip, data.size());
    for (size_t i = 0; i < data.size(); i++) {
        float px = graphX + i * xStep;
        float py = graphTop + (1.0f - data[i] / maxVal) * graphHeight;
        lines[i].position = {px, py};
        lines[i].color = color;
    }
    window.draw(lines);
}

void Renderer::handleEvent(const sf::Event& event, sf::RenderWindow& window, Simulation& sim) {
    if (!uiInitialized) {
        setupUI(sim);
    }
    
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2f mousePos(static_cast<float>(mousePressed->position.x),
                              static_cast<float>(mousePressed->position.y));
        handleMouseClick(mousePos, mousePressed->button, sim);
    }
    else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseReleased->position.x),
                              static_cast<float>(mouseReleased->position.y));
        handleMouseRelease(mousePos, sim);
    }
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos(static_cast<float>(mouseMoved->position.x),
                              static_cast<float>(mouseMoved->position.y));
        handleMouseMove(mousePos, sim);
    }
    else if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        handleKeyPress(keyPressed->code, sim);
    }
    else if (event.is<sf::Event::Resized>()) {
        // Window resized - update view
        sf::Vector2u newSize = window.getSize();
        sf::View view(sf::FloatRect({0, 0}, {static_cast<float>(newSize.x), static_cast<float>(newSize.y)}));
        window.setView(view);
        
        windowWidth = static_cast<int>(newSize.x);
        windowHeight = static_cast<int>(newSize.y);
        recalculateLayout();
        uiInitialized = false;
    }
}

void Renderer::handleMouseClick(sf::Vector2f mousePos, sf::Mouse::Button button, Simulation& sim) {
    sf::FloatRect worldBounds(worldOffset, {worldDisplaySize, worldDisplaySize});
    
    // Handle obstacle editing mode
    if (editingObstacles && worldBounds.contains(mousePos)) {
        sf::Vector2f worldPos = (mousePos - worldOffset) / worldScale;
        
        if (button == sf::Mouse::Button::Right) {
            // Right-click: erase obstacle under cursor
            for (auto it = sim.obstacles.begin(); it != sim.obstacles.end(); ) {
                if (it->contains(worldPos)) {
                    it = sim.obstacles.erase(it);
                } else {
                    ++it;
                }
            }
            return;
        } else if (button == sf::Mouse::Button::Left) {
            // Left-click: start drawing new obstacle
            isDrawing = true;
            isErasing = false;
            drawStart = worldPos;
            return;
        }
    }
    
    // Handle survival zone editing mode
    if (editingSurvivalZones && worldBounds.contains(mousePos)) {
        sf::Vector2f worldPos = (mousePos - worldOffset) / worldScale;
        
        if (button == sf::Mouse::Button::Right) {
            // Right-click: erase zone under cursor
            for (auto it = sim.survivalZones.begin(); it != sim.survivalZones.end(); ) {
                if (it->contains(worldPos)) {
                    it = sim.survivalZones.erase(it);
                } else {
                    ++it;
                }
            }
            return;
        } else if (button == sf::Mouse::Button::Left) {
            // Left-click: start drawing new zone
            isDrawing = true;
            isErasing = false;
            drawStart = worldPos;
            return;
        }
    }
    
    if (button != sf::Mouse::Button::Left) return;
    
    // Buttons
    for (size_t i = 0; i < buttons.size(); i++) {
        if (buttons[i].handleClick(mousePos)) {
            if (i == 0) {
                sim.paused = !sim.paused;
            } else if (i == 1) {
                sim.reset();
                uiInitialized = false;
            } else if (i == 2) {
                // Reset Settings button
                sim.config.resetToDefaults();
                uiInitialized = false;
            } else if (i == 3) {
                // Edit Obstacles button
                editingObstacles = !editingObstacles;
                editingSurvivalZones = false;  // Only one edit mode at a time
                if (editingObstacles) {
                    sim.config.obstaclesEnabled = true;
                }
            } else if (i == 4) {
                // Edit Survival Zones button
                editingSurvivalZones = !editingSurvivalZones;
                editingObstacles = false;  // Only one edit mode at a time
                if (editingSurvivalZones) {
                    sim.config.survivalZoneEnabled = true;
                }
            }
            return;
        }
    }
    
    // Sliders
    for (size_t i = 0; i < sliders.size(); i++) {
        if (sliders[i].handleClick(mousePos)) {
            isDraggingSlider = true;
            activeSliderIndex = static_cast<int>(i);
            return;
        }
    }
    
    // Toggles
    for (Toggle& toggle : toggles) {
        if (toggle.handleClick(mousePos)) {
            return;
        }
    }
    
    // Bottom bar toggles
    if (bottomBarToggles[0].contains(mousePos)) {
        sim.config.showSenseRange = !sim.config.showSenseRange;
        return;
    }
    if (bottomBarToggles[1].contains(mousePos)) {
        sim.config.showHungerColors = !sim.config.showHungerColors;
        return;
    }
    
    // World click (select peep) - only if not in edit mode
    if (!editingObstacles && !editingSurvivalZones && worldBounds.contains(mousePos)) {
        sf::Vector2f worldPos = (mousePos - worldOffset) / worldScale;
        
        float minDist = sim.config.peepSize * 4;
        int closestPeep = -1;
        
        for (size_t i = 0; i < sim.peeps.size(); i++) {
            if (!sim.peeps[i].alive) continue;
            
            sf::Vector2f diff = sim.peeps[i].position - worldPos;
            float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            
            if (dist < minDist) {
                minDist = dist;
                closestPeep = static_cast<int>(i);
            }
        }
        
        sim.selectedPeep = closestPeep;
    }
}

void Renderer::handleMouseMove(sf::Vector2f mousePos, Simulation& sim) {
    (void)sim;
    
    if (isDraggingSlider && activeSliderIndex >= 0 && 
        activeSliderIndex < static_cast<int>(sliders.size())) {
        sliders[activeSliderIndex].handleDrag(mousePos);
    }
}

void Renderer::handleMouseRelease(sf::Vector2f mousePos, Simulation& sim) {
    // Finish drawing obstacle
    if (isDrawing && editingObstacles) {
        sf::Vector2f worldEnd = (mousePos - worldOffset) / worldScale;
        
        // Calculate rectangle from start to current
        float x = std::min(drawStart.x, worldEnd.x);
        float y = std::min(drawStart.y, worldEnd.y);
        float w = std::abs(worldEnd.x - drawStart.x);
        float h = std::abs(worldEnd.y - drawStart.y);
        
        // Only create if large enough (minimum 10x10)
        if (w > 10 && h > 10) {
            // Clamp to world bounds
            x = std::max(0.0f, std::min(x, sim.config.worldSize - w));
            y = std::max(0.0f, std::min(y, sim.config.worldSize - h));
            
            sim.obstacles.push_back(Obstacle(x, y, w, h));
        }
        
        isDrawing = false;
    }
    
    // Finish drawing survival zone
    if (isDrawing && editingSurvivalZones) {
        sf::Vector2f worldEnd = (mousePos - worldOffset) / worldScale;
        
        // Calculate rectangle from start to current
        float x = std::min(drawStart.x, worldEnd.x);
        float y = std::min(drawStart.y, worldEnd.y);
        float w = std::abs(worldEnd.x - drawStart.x);
        float h = std::abs(worldEnd.y - drawStart.y);
        
        // Only create if large enough (minimum 10x10)
        if (w > 10 && h > 10) {
            // Clamp to world bounds
            x = std::max(0.0f, std::min(x, sim.config.worldSize - w));
            y = std::max(0.0f, std::min(y, sim.config.worldSize - h));
            
            sim.survivalZones.push_back(SurvivalZone(x, y, w, h));
        }
        
        isDrawing = false;
    }
    
    if (isDraggingSlider && activeSliderIndex >= 0 && 
        activeSliderIndex < static_cast<int>(sliders.size())) {
        sliders[activeSliderIndex].dragging = false;
    }
    isDraggingSlider = false;
    activeSliderIndex = -1;
}

void Renderer::handleKeyPress(sf::Keyboard::Key key, Simulation& sim) {
    switch (key) {
        case sf::Keyboard::Key::Space:
            sim.paused = !sim.paused;
            break;
        case sf::Keyboard::Key::R:
            sim.reset();
            uiInitialized = false;
            break;
        case sf::Keyboard::Key::E:
            // Toggle obstacle edit mode
            editingObstacles = !editingObstacles;
            editingSurvivalZones = false;
            if (editingObstacles) {
                sim.config.obstaclesEnabled = true;
            }
            isDrawing = false;
            break;
        case sf::Keyboard::Key::Z:
            // Toggle survival zone edit mode
            editingSurvivalZones = !editingSurvivalZones;
            editingObstacles = false;
            if (editingSurvivalZones) {
                sim.config.survivalZoneEnabled = true;
            }
            isDrawing = false;
            break;
        case sf::Keyboard::Key::Escape:
            if (editingObstacles || editingSurvivalZones) {
                editingObstacles = false;
                editingSurvivalZones = false;
                isDrawing = false;
            } else {
                sim.selectedPeep = -1;
            }
            break;
        case sf::Keyboard::Key::Up:
            sim.speedMultiplier = std::min(100.0f, sim.speedMultiplier < 10 ? sim.speedMultiplier + 1 : sim.speedMultiplier * 1.5f);
            break;
        case sf::Keyboard::Key::Down:
            sim.speedMultiplier = std::max(1.0f, sim.speedMultiplier > 10 ? sim.speedMultiplier / 1.5f : sim.speedMultiplier - 1);
            break;
        case sf::Keyboard::Key::Num1:
            sim.speedMultiplier = 1.0f;
            break;
        case sf::Keyboard::Key::Num2:
            sim.speedMultiplier = 2.0f;
            break;
        case sf::Keyboard::Key::Num5:
            sim.speedMultiplier = 5.0f;
            break;
        case sf::Keyboard::Key::Num0:
            sim.speedMultiplier = 10.0f;
            break;
        default:
            break;
    }
}

// ============================================================================
// MAIN LOOP
// ============================================================================

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
        
        // Cap to prevent issues when window loses focus
        if (dt > 0.1f) dt = 0.1f;
        
        sim.update(dt);
        renderer.render(window, sim);
    }
}
