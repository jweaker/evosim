// render.hpp - Rendering and UI

#pragma once
#include "simulation.hpp"
#include <SFML/Graphics.hpp>
#include <string>

// ============================================================================
// UI HELPER STRUCTURES
// ============================================================================

// A simple slider for adjusting values
struct Slider {
    sf::FloatRect bounds;
    float minValue;
    float maxValue;
    float* value;
    std::string label;
    bool isInteger;
    bool isLogarithmic;  // Use logarithmic scale
    bool dragging;
    
    Slider();
    Slider(float x, float y, float width, float height,
           float minVal, float maxVal, float* val,
           const std::string& lbl, bool isInt = false, bool isLog = false);
    
    bool handleClick(sf::Vector2f mousePos);
    void handleDrag(sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window, const sf::Font& font);
};

// A toggle button
struct Toggle {
    sf::FloatRect bounds;
    bool* value;
    std::string label;
    
    Toggle();
    Toggle(float x, float y, float width, float height,
           bool* val, const std::string& lbl);
    
    bool handleClick(sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window, const sf::Font& font);
};

// A clickable button
struct Button {
    sf::FloatRect bounds;
    std::string label;
    
    Button();
    Button(float x, float y, float width, float height, const std::string& lbl);
    
    bool handleClick(sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window, const sf::Font& font, bool highlighted = false);
};

// ============================================================================
// RENDERER CLASS
// ============================================================================

class Renderer {
public:
    Renderer();
    
    // Initialize with window dimensions
    void initialize(int windowWidth, int windowHeight);
    
    // Load font
    bool loadFont(const std::string& fontPath);
    
    // Main render function
    void render(sf::RenderWindow& window, Simulation& sim);
    
    // Handle input events
    void handleEvent(const sf::Event& event, sf::RenderWindow& window, Simulation& sim);
    
    // Get world offset (for converting mouse coords to world coords)
    sf::Vector2f getWorldOffset() const { return worldOffset; }
    float getWorldScale() const { return worldScale; }
    
private:
    // Drawing functions
    void drawWorld(sf::RenderWindow& window, Simulation& sim);
    void drawGrid(sf::RenderWindow& window, Simulation& sim);
    void drawPeeps(sf::RenderWindow& window, Simulation& sim);
    void drawFood(sf::RenderWindow& window, Simulation& sim);
    void drawObstacles(sf::RenderWindow& window, Simulation& sim);
    void drawSurvivalZone(sf::RenderWindow& window, Simulation& sim);
    void drawSelectedPeep(sf::RenderWindow& window, Simulation& sim);
    
    void drawUI(sf::RenderWindow& window, Simulation& sim);
    void drawStats(sf::RenderWindow& window, Simulation& sim);
    void drawBottomBar(sf::RenderWindow& window, Simulation& sim);
    void drawGraph(sf::RenderWindow& window, const std::vector<float>& data,
                   sf::FloatRect bounds, sf::Color color, const std::string& label);
    void drawControls(sf::RenderWindow& window, Simulation& sim);
    void drawPeepInfo(sf::RenderWindow& window, Simulation& sim);
    
    // Input handling
    void handleMouseClick(sf::Vector2f mousePos, sf::Mouse::Button button, Simulation& sim);
    void handleMouseMove(sf::Vector2f mousePos, Simulation& sim);
    void handleMouseRelease(sf::Vector2f mousePos, Simulation& sim);
    void handleKeyPress(sf::Keyboard::Key key, Simulation& sim);
    
    // Setup UI elements
    void setupUI(Simulation& sim);
    
    // Font
    sf::Font font;
    bool fontLoaded;
    
    // Layout
    float uiPanelWidth;
    float bottomBarHeight;
    float worldDisplaySize;
    sf::Vector2f worldOffset;
    float worldScale;
    
    // UI elements
    std::vector<Slider> sliders;
    std::vector<Toggle> toggles;
    std::vector<Button> buttons;
    bool uiInitialized;
    float controlsBottomY;  // Track where controls end for graph positioning
    
    // Mouse state
    bool isDraggingSlider;
    int activeSliderIndex;
    
    // Window dimensions (for responsive layout)
    int windowWidth;
    int windowHeight;
    
    // DNA panel state
    bool showDNAPanel;
    
    // Editing modes
    bool editingObstacles;
    bool editingSurvivalZones;
    bool isDrawing;           // Currently drawing/erasing
    bool isErasing;           // Right-click = erase, Left-click = draw
    sf::Vector2f drawStart;   // Start position of current shape being drawn
    
    // Bottom bar toggle bounds for click detection
    sf::FloatRect bottomBarToggles[2];  // 0 = Show Sense Range, 1 = Color by Hunger
    
    // Layout helpers
    void recalculateLayout();
    void drawDNAPanel(sf::RenderWindow& window, Simulation& sim);
    void drawEditModeOverlay(sf::RenderWindow& window, Simulation& sim);
    void drawSurvivalZones(sf::RenderWindow& window, Simulation& sim);
};

// ============================================================================
// MAIN LOOP
// ============================================================================

void runMainLoop(sf::RenderWindow& window, Simulation& sim, Renderer& renderer);
