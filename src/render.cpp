
#include "render.hpp"
#include "color_utils.hpp"
#include <algorithm>
#include <sstream>

static const sf::FloatRect PAUSE_BTN({10, 740}, {80, 40});
static const sf::FloatRect RESET_BTN({100, 740}, {80, 40});

Renderer::Renderer(sf::RenderWindow &win, const sf::Font &f)
    : window(win), statsText(f, "", 24), font(f) {}

void drawSurvivalZone(sf::RenderWindow &window, SurvivalMode mode,
                      sf::Vector2f offset) {
  sf::RectangleShape rect;
  rect.setFillColor(sf::Color(0, 255, 0, 60));

  float half = WorldSize / 2.f;

  switch (mode) {
  case SurvivalMode::EastHalf:
    rect.setSize({half, WorldSize});
    rect.setPosition(offset + sf::Vector2f(half, 0));
    window.draw(rect);
    break;
  case SurvivalMode::WestHalf:
    rect.setSize({half, WorldSize});
    rect.setPosition(offset);
    window.draw(rect);
    break;
  case SurvivalMode::QuadrantNE:
    rect.setSize({half, half});
    rect.setPosition(offset + sf::Vector2f(half, 0));
    window.draw(rect);
    break;
  case SurvivalMode::QuadrantNW:
    rect.setSize({half, half});
    rect.setPosition(offset);
    window.draw(rect);
    break;
  case SurvivalMode::QuadrantSE:
    rect.setSize({half, half});
    rect.setPosition(offset + sf::Vector2f(half, half));
    window.draw(rect);
    break;
  case SurvivalMode::QuadrantSW:
    rect.setSize({half, half});
    rect.setPosition(offset + sf::Vector2f(0, half));
    window.draw(rect);
    break;
  case SurvivalMode::FourCorners: {
    float quarter = WorldSize / 4.f;
    sf::RectangleShape corner({quarter, quarter});
    corner.setFillColor(sf::Color(0, 255, 0, 60));
    std::array<sf::Vector2f, 4> positions = {
        offset, offset + sf::Vector2f(WorldSize - quarter, 0),
        offset + sf::Vector2f(0, WorldSize - quarter),
        offset + sf::Vector2f(WorldSize - quarter, WorldSize - quarter)};
    for (const auto &pos : positions) {
      corner.setPosition(pos);
      window.draw(corner);
    }
    break;
  }
  }
}
void Renderer::drawLegend(SurvivalMode current) {
  // List of (key, name, enum)
  static const std::array<std::pair<int, const char *>, 7> items = {
      {{1, "East Half"},
       {2, "West Half"},
       {3, "NE Quadrant"},
       {4, "NW Quadrant"},
       {5, "SE Quadrant"},
       {6, "SW Quadrant"},
       {7, "Four Corners"}}};
  sf::Text txt(font, "", 18);

  float x = 10.f, y = 300.f;
  for (size_t i = 0; i < items.size(); ++i) {
    const auto &[key, name] = items[i];
    SurvivalMode mode = static_cast<SurvivalMode>(i);
    // draw "1: East Half", etc.
    txt.setString(std::to_string(key) + ": " + name);
    txt.setPosition({x, y + i * 22.f});
    if (mode == current) {
      txt.setFillColor(sf::Color::Yellow);
    } else {
      txt.setFillColor(sf::Color(200, 200, 200));
    }
    window.draw(txt);
  }
}

void Renderer::draw(const Simulation &sim, bool speedUp, SurvivalMode mode,
                    bool paused) {
  window.clear({100, 100, 100});

  // 1) draw buttons
  sf::RectangleShape btn;
  sf::Text label(font, "", 18);

  // Pause button

  btn.setSize({PAUSE_BTN.size.x, PAUSE_BTN.size.y});
  btn.setPosition({PAUSE_BTN.position.x, PAUSE_BTN.position.y});
  btn.setFillColor(paused ? sf::Color(200, 200, 0) : sf::Color(100, 100, 100));
  window.draw(btn);
  label.setString(paused ? "Resume" : "Pause");
  label.setPosition({PAUSE_BTN.position.x + 10, PAUSE_BTN.position.y + 8});
  window.draw(label);

  // Reset button
  btn.setSize({RESET_BTN.size.x, RESET_BTN.size.y});
  btn.setPosition({RESET_BTN.position.x, RESET_BTN.position.y});
  btn.setFillColor(sf::Color(100, 100, 100));
  window.draw(btn);
  label.setString("Reset");
  label.setPosition({RESET_BTN.position.x + 10, RESET_BTN.position.y + 8});
  window.draw(label);
  const auto sz = window.getSize();
  sf::RectangleShape frame{{524, 524}}, board{{512, 512}};
  frame.setFillColor(sf::Color::Black);
  board.setFillColor(sf::Color::White);
  frame.setPosition({(sz.x - 524) / 2.f, (sz.y - 524) / 2.f});
  board.setPosition({(sz.x - 512) / 2.f, (sz.y - 512) / 2.f});
  window.draw(frame);
  window.draw(board);

  auto offset = sf::Vector2f((sz.x - 512) / 2.f, (sz.y - 512) / 2.f);
  drawSurvivalZone(window, mode, offset);
  drawLegend(mode);

  // peeps
  sf::CircleShape dot{3.f};
  dot.setOrigin({3, 3});
  for (const auto &p : sim.peeps) {
    dot.setPosition(p.position + offset);
    dot.setFillColor(genomeToColor(p.genome));
    window.draw(dot);
  }

  // stats text
  std::ostringstream ss;
  ss << " Gen: " << sim.generation
     << "   -   Time: " << int(GenerationTime - sim.timer)
     << "   -   Survival: "
     << (float)((int)(10000.f *
                      std::count_if(sim.peeps.begin(), sim.peeps.end(),
                                    [&](const auto &p) {
                                      return sim.isSurvivor(p, mode);
                                    }) /
                      float(PeepCount))) /
            100.f
     << "%\n (Press Space To Speed Up)\n"
     << (speedUp ? " Speed x10" : "");
  statsText.setString(ss.str());
  window.draw(statsText);
  window.display();
}

void runLoop(sf::RenderWindow &window, Simulation &sim, Renderer &renderer,
             InputHandler &input) {
  sf::Clock clock;
  while (window.isOpen()) {
    std::optional<sf::Event> evt;
    while ((evt = window.pollEvent())) {
      const auto &e = *evt;
      if (e.is<sf::Event::Closed>()) {
        window.close();
        break;
      }
      input.processEvent(e);

      // mouse‐click handling for our buttons:
      if (e.is<sf::Event::MouseButtonPressed>() &&
          e.getIf<sf::Event::MouseButtonPressed>()->button ==
              sf::Mouse::Button::Left) {
        auto m = e.getIf<sf::Event::MouseButtonPressed>()->position;
        sf::Vector2f pos{float(m.x), float(m.y)};

        if (PAUSE_BTN.contains(pos)) {
          input.paused = !input.paused;
        } else if (RESET_BTN.contains(pos)) {
          input.resetRequested = true;
        }
      }
    }

    // handle reset
    if (input.resetRequested) {
      sim.initPeeps();
      input.resetRequested = false;
    }

    float dt = clock.restart().asSeconds();
    if (!input.paused) {
      sim.update(dt, input.speedUp, input.mode);
    }
    renderer.draw(sim, input.speedUp, input.mode, input.paused);
  }
}
