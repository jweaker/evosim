#pragma once
#include "simulation.hpp"
#include "ui/widgets.hpp"
#include <SFML/Graphics.hpp>
#include <vector>

class Renderer {
public:
    Renderer();
    
    void initialize(int width, int height);
    bool loadFont(const std::string& path);
    void render(sf::RenderWindow& window, Simulation& sim);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window, Simulation& sim);
    
    sf::Vector2f getWorldOffset() const { return worldOffset; }
    float getWorldScale() const { return worldScale; }

private:
    sf::Font font;
    bool fontLoaded;
    
    float uiPanelWidth;
    float bottomBarHeight;
    float worldDisplaySize;
    sf::Vector2f worldOffset;
    float worldScale;
    
    std::vector<Slider> sliders;
    std::vector<Toggle> toggles;
    std::vector<Button> buttons;
    bool uiReady;
    float controlsBottom;
    
    bool draggingSlider;
    int activeSlider;
    int windowWidth;
    int windowHeight;
    
    bool editingObstacles;
    bool editingSurvivalZones;
    bool editingSpawnZones;
    bool isDrawing;
    bool isErasing;
    sf::Vector2f drawStart;
    
    sf::FloatRect bottomToggles[2];
    
    void recalculateLayout();
    void setupUI(Simulation& sim);
    
    void drawWorld(sf::RenderWindow& window, Simulation& sim);
    void drawGrid(sf::RenderWindow& window, Simulation& sim);
    void drawPeeps(sf::RenderWindow& window, Simulation& sim);
    void drawFood(sf::RenderWindow& window, Simulation& sim);
    void drawObstacles(sf::RenderWindow& window, Simulation& sim);
    void drawSurvivalZones(sf::RenderWindow& window, Simulation& sim);
    void drawSpawnZones(sf::RenderWindow& window, Simulation& sim);
    void drawUI(sf::RenderWindow& window, Simulation& sim);
    void drawStats(sf::RenderWindow& window, Simulation& sim);
    void drawControls(sf::RenderWindow& window, Simulation& sim);
    void drawBottomBar(sf::RenderWindow& window, Simulation& sim);
    void drawGraph(sf::RenderWindow& window, const std::vector<float>& data,
                   sf::FloatRect bounds, sf::Color color, const std::string& label);
    void drawDNAPanel(sf::RenderWindow& window, Simulation& sim);
    void drawEditOverlay(sf::RenderWindow& window, Simulation& sim);
    
    void handleMouseClick(sf::Vector2f pos, sf::Mouse::Button btn, Simulation& sim);
    void handleMouseMove(sf::Vector2f pos, Simulation& sim);
    void handleMouseRelease(sf::Vector2f pos, Simulation& sim);
    void handleKeyPress(sf::Keyboard::Key key, Simulation& sim);
};

void runMainLoop(sf::RenderWindow& window, Simulation& sim, Renderer& renderer);
