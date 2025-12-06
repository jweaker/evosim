// renderer.cpp - Core Renderer class methods
// Constructor, initialization, layout, and main render loop

#include "../render.hpp"
#include <algorithm>

Renderer::Renderer()
    : fontLoaded(false)
    , uiPanelWidth(250.0f)
    , bottomBarHeight(68.0f)
    , worldDisplaySize(600.0f)
    , worldScale(1.0f)
    , uiReady(false)
    , controlsBottom(0.0f)
    , draggingSlider(false)
    , activeSlider(-1)
    , windowWidth(1600)
    , windowHeight(1000)
    , editingObstacles(false)
    , editingSurvivalZones(false)
    , editingSpawnZones(false)
    , isDrawing(false)
    , isErasing(false) {}

void Renderer::initialize(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    recalculateLayout();
}

void Renderer::recalculateLayout() {
    uiPanelWidth = std::max(220.0f, std::min(280.0f, windowWidth * 0.18f));
    bottomBarHeight = 68.0f;
    
    float margin = 8.0f;
    float availW = windowWidth - uiPanelWidth - margin * 2;
    float availH = windowHeight - bottomBarHeight - margin;
    worldDisplaySize = std::min(availW, availH);
    
    worldOffset.x = uiPanelWidth + (availW - worldDisplaySize) / 2 + margin;
    worldOffset.y = (availH - worldDisplaySize) / 2;
}

bool Renderer::loadFont(const std::string& path) {
    fontLoaded = font.openFromFile(path);
    return fontLoaded;
}

void Renderer::setupUI(Simulation& sim) {
    sliders.clear();
    toggles.clear();
    buttons.clear();
    
    float pad = 10.0f;
    float x = pad;
    float w = uiPanelWidth - pad * 2;
    float sliderH = 32.0f;
    float toggleH = 26.0f;
    float buttonH = 28.0f;
    float secGap = 14.0f;
    float itemGap = 4.0f;
    float y = 52.0f;
    
    float btnGap = 6.0f;
    float btnW = (w - btnGap * 2) / 3;
    buttons.emplace_back(x, y, btnW, buttonH, "Pause");
    buttons.emplace_back(x + btnW + btnGap, y, btnW, buttonH, "Reset");
    buttons.emplace_back(x + (btnW + btnGap) * 2, y, btnW, buttonH, "Defaults");
    y += buttonH + secGap;
    
    sliders.emplace_back(x, y, w, sliderH, 1.0f, 100000.0f, &sim.speedMultiplier, "Speed", true, true);
    y += sliderH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 5.0f, 60.0f, &sim.config.generationTime, "Gen Time (sec)");
    y += sliderH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 20.0f, 1000.0f, &sim.config.initialPopulation, "Population*", true);
    y += sliderH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 50.0f, 500.0f, &sim.config.peepSpeed, "Peep Speed", true);
    y += sliderH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 20.0f, 300.0f, &sim.config.peepSenseRange, "Sense Range", true);
    y += sliderH + secGap;
    
    sliders.emplace_back(x, y, w, sliderH, 1.0f, 128.0f, &sim.config.genesPerPeep, "Genes*", true);
    y += sliderH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 1.0f, 32.0f, &sim.config.hiddenNeurons, "Hidden Neurons*", true);
    y += sliderH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 0.0f, 0.5f, &sim.config.mutationRate, "Mutation Rate");
    y += sliderH + itemGap;
    toggles.emplace_back(x, y, w, toggleH, &sim.config.useCrossover, "Sexual Reproduction");
    y += toggleH + secGap;
    
    toggles.emplace_back(x, y, w, toggleH, &sim.config.hungerEnabled, "Hunger System");
    y += toggleH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 0.5f, 30.0f, &sim.config.hungerDecayRate, "Hunger Decay Rate");
    y += sliderH + itemGap;
    sliders.emplace_back(x, y, w, sliderH, 20.0f, 500.0f, &sim.config.foodCount, "Food Count", true);
    y += sliderH + secGap;
    
    toggles.emplace_back(x, y, w, toggleH, &sim.config.obstaclesEnabled, "Obstacles");
    y += toggleH + itemGap;
    buttons.emplace_back(x, y, w, buttonH, "Edit Obstacles");
    y += buttonH + itemGap;
    toggles.emplace_back(x, y, w, toggleH, &sim.config.survivalZoneEnabled, "Survival Zones");
    y += toggleH + itemGap;
    buttons.emplace_back(x, y, w, buttonH, "Edit Zones");
    y += buttonH + itemGap;
    toggles.emplace_back(x, y, w, toggleH, &sim.config.spawnZonesEnabled, "Spawn Zones");
    y += toggleH + itemGap;
    buttons.emplace_back(x, y, w, buttonH, "Edit Spawn Area");
    y += buttonH + secGap;
    
    float presetW = (w - btnGap) / 2;
    float presetH = 24.0f;
    buttons.emplace_back(x, y, presetW, presetH, "Migration");
    buttons.emplace_back(x + presetW + btnGap, y, presetW, presetH, "Foraging");
    y += presetH + itemGap;
    buttons.emplace_back(x, y, presetW, presetH, "Maze");
    buttons.emplace_back(x + presetW + btnGap, y, presetW, presetH, "Hunger Games");
    y += presetH + itemGap;
    buttons.emplace_back(x, y, presetW, presetH, "Big Brains");
    buttons.emplace_back(x + presetW + btnGap, y, presetW, presetH, "Speed Demons");
    y += presetH + 8;
    
    controlsBottom = y + 14;
    uiReady = true;
}

void Renderer::render(sf::RenderWindow& window, Simulation& sim) {
    sf::Vector2u sz = window.getSize();
    if ((int)sz.x != windowWidth || (int)sz.y != windowHeight) {
        windowWidth = (int)sz.x;
        windowHeight = (int)sz.y;
        recalculateLayout();
        uiReady = false;
    }
    
    if (!uiReady) setupUI(sim);
    
    window.clear(sf::Color(25, 25, 30));
    drawWorld(window, sim);
    drawUI(window, sim);
    
    if (editingObstacles || editingSurvivalZones || editingSpawnZones) {
        drawEditOverlay(window, sim);
    }
    
    if (sim.selectedPeep >= 0 && sim.selectedPeep < (int)sim.peeps.size()) {
        drawDNAPanel(window, sim);
    }
    
    window.display();
}
