// draw_ui.cpp - UI drawing methods
// Draws sidebar, stats, controls, bottom bar, graphs, DNA panel, edit overlay

#include "../render.hpp"
#include "../neural/gene.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

void Renderer::drawUI(sf::RenderWindow& window, Simulation& sim) {
    sf::RectangleShape sidebar({uiPanelWidth, (float)windowHeight});
    sidebar.setFillColor(sf::Color(32, 34, 40));
    window.draw(sidebar);
    
    sf::RectangleShape sidebarBorder({2, (float)windowHeight});
    sidebarBorder.setPosition({uiPanelWidth - 2, 0});
    sidebarBorder.setFillColor(sf::Color(50, 52, 60));
    window.draw(sidebarBorder);
    
    float barY = (float)windowHeight - bottomBarHeight;
    sf::RectangleShape bottom({(float)windowWidth - uiPanelWidth, bottomBarHeight});
    bottom.setPosition({uiPanelWidth, barY});
    bottom.setFillColor(sf::Color(32, 34, 40));
    window.draw(bottom);
    
    sf::RectangleShape bottomBorder({(float)windowWidth - uiPanelWidth, 2});
    bottomBorder.setPosition({uiPanelWidth, barY});
    bottomBorder.setFillColor(sf::Color(50, 52, 60));
    window.draw(bottomBorder);
    
    if (!fontLoaded) return;
    
    drawStats(window, sim);
    drawControls(window, sim);
    drawBottomBar(window, sim);
}

void Renderer::drawStats(sf::RenderWindow& window, Simulation& sim) {
    float x = 12.0f, y = 8.0f;
    
    sf::Text title(font, "Evolution Sim", 16);
    title.setPosition({x, y});
    title.setFillColor(sf::Color(220, 220, 225));
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    
    float timeLeft = std::max(0.0f, sim.config.generationTime - sim.generationTimer);
    
    if (sim.paused) {
        sf::Text pause(font, "PAUSED", 11);
        pause.setPosition({x + 120.0f, y + 2.0f});
        pause.setFillColor(sf::Color(255, 180, 80));
        window.draw(pause);
    } else {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << timeLeft << "s";
        sf::Text time(font, ss.str(), 11);
        time.setPosition({x + 120.0f, y + 2.0f});
        time.setFillColor(sf::Color(140, 140, 145));
        window.draw(time);
    }
    
    sf::RectangleShape sep({uiPanelWidth - 20.0f, 1.0f});
    sep.setPosition({x, y + 24.0f});
    sep.setFillColor(sf::Color(50, 52, 60));
    window.draw(sep);
}

void Renderer::drawControls(sf::RenderWindow& window, Simulation& sim) {
    if (!buttons.empty()) buttons[0].label = sim.paused ? "Resume" : "Pause";
    if (buttons.size() > 3) buttons[3].label = editingObstacles ? "Done Editing" : "Edit Obstacles";
    if (buttons.size() > 4) buttons[4].label = editingSurvivalZones ? "Done Editing" : "Edit Zones";
    if (buttons.size() > 5) buttons[5].label = editingSpawnZones ? "Done Editing" : "Edit Spawn Area";
    
    for (size_t i = 0; i < buttons.size(); i++) {
        bool hl = (i == 3 && editingObstacles) || (i == 4 && editingSurvivalZones) || (i == 5 && editingSpawnZones);
        buttons[i].draw(window, font, hl);
    }
    
    for (auto& s : sliders) s.draw(window, font);
    for (auto& t : toggles) t.draw(window, font);
    
    sf::Text hint(font, "* = needs Reset to apply", 10);
    hint.setPosition({10.0f, controlsBottom - 12.0f});
    hint.setFillColor(sf::Color(120, 120, 125));
    window.draw(hint);
}

void Renderer::drawBottomBar(sf::RenderWindow& window, Simulation& sim) {
    if (!fontLoaded) return;
    
    float barY = (float)windowHeight - bottomBarHeight;
    float padX = 12.0f, padY = 8.0f;
    float contentY = barY + padY;
    float contentH = bottomBarHeight - padY * 2;
    
    float toggleW = 145.0f, toggleH = 24.0f, toggleGap = 4.0f;
    float toggleX = (float)windowWidth - toggleW - padX * 2;
    float toggleY = contentY + 2.0f;
    
    auto drawToggle = [&](bool on, const std::string& label, int idx) {
        float sw = 32.0f, sh = 16.0f;
        float sx = toggleX + toggleW - sw;
        float sy = toggleY + (toggleH - sh) / 2;
        
        bottomToggles[idx] = sf::FloatRect({toggleX, toggleY}, {toggleW, toggleH});
        
        sf::RectangleShape bg({sw, sh});
        bg.setPosition({sx, sy});
        bg.setFillColor(on ? sf::Color(70, 150, 70) : sf::Color(50, 50, 55));
        bg.setOutlineColor(sf::Color(60, 60, 65));
        bg.setOutlineThickness(1);
        window.draw(bg);
        
        float kr = 6.0f;
        sf::CircleShape knob(kr);
        knob.setOrigin({kr, kr});
        knob.setPosition({on ? sx + sw - kr - 2 : sx + kr + 2, sy + sh / 2});
        knob.setFillColor(sf::Color::White);
        window.draw(knob);
        
        sf::Text lbl(font, label, 13);
        lbl.setPosition({toggleX, toggleY + (toggleH - 13) / 2});
        lbl.setFillColor(sf::Color(200, 200, 205));
        window.draw(lbl);
    };
    
    drawToggle(sim.config.showSenseRange, "Sense Range", 0);
    toggleY += toggleH + toggleGap;
    drawToggle(sim.config.showHungerColors, "Hunger Colors", 1);
    
    float statsW = 110.0f, statsX = toggleX - statsW - padX;
    float lineH = 16.0f, statsY = contentY + 4.0f;
    
    std::ostringstream ss;
    ss << "Gen " << sim.currentGeneration;
    sf::Text gen(font, ss.str(), 13);
    gen.setPosition({statsX, statsY});
    gen.setFillColor(sf::Color(200, 200, 205));
    window.draw(gen);
    
    ss.str("");
    ss << sim.stats.aliveCount << "/" << sim.peeps.size();
    sf::Text pop(font, ss.str(), 12);
    pop.setPosition({statsX, statsY + lineH + 2.0f});
    pop.setFillColor(sf::Color(160, 160, 165));
    window.draw(pop);
    
    if (!sim.stats.survivalHistory.empty()) {
        float last = sim.stats.survivalHistory.back();
        ss.str("");
        ss << std::fixed << std::setprecision(0) << (last * 100.0f) << "%";
        sf::Text surv(font, ss.str(), 12);
        surv.setPosition({statsX + 55.0f, statsY + lineH + 2.0f});
        surv.setFillColor(last >= 0.5f ? sf::Color(100, 200, 120) : 
                          last >= 0.2f ? sf::Color(200, 200, 100) : sf::Color(200, 120, 100));
        window.draw(surv);
    }
    
    float graphX = uiPanelWidth + padX;
    float graphW = statsX - graphX - padX;
    drawGraph(window, sim.stats.survivalHistory, 
              sf::FloatRect({graphX, contentY}, {graphW, contentH}),
              sf::Color(100, 200, 150), "Survival Rate");
    
    sf::Text hints(font, "Space:Pause  R:Reset  E:Obstacles  Z:Zones  S:Spawn  1/2/5/0:Speed", 10);
    hints.setPosition({graphX + 5, contentY + contentH - 12.0f});
    hints.setFillColor(sf::Color(100, 100, 105));
    window.draw(hints);
}

void Renderer::drawGraph(sf::RenderWindow& window, const std::vector<float>& data,
                         sf::FloatRect bounds, sf::Color color, const std::string& label) {
    if (data.empty()) return;
    
    float leftMargin = 30.0f;
    float graphX = bounds.position.x + leftMargin;
    float graphW = bounds.size.x - leftMargin;
    
    sf::RectangleShape bg(bounds.size);
    bg.setPosition(bounds.position);
    bg.setFillColor(sf::Color(25, 27, 32));
    bg.setOutlineColor(sf::Color(50, 52, 58));
    bg.setOutlineThickness(1);
    window.draw(bg);
    
    sf::Text lbl(font, label, 10);
    lbl.setPosition({graphX + 5, bounds.position.y + 2});
    lbl.setFillColor(sf::Color(120, 120, 125));
    window.draw(lbl);
    
    float graphTop = bounds.position.y + 15;
    float graphBottom = bounds.position.y + bounds.size.y;
    float graphH = graphBottom - graphTop;
    
    sf::Text y100(font, "100%", 9);
    y100.setPosition({bounds.position.x + 2, graphTop - 3});
    y100.setFillColor(sf::Color(90, 90, 95));
    window.draw(y100);
    
    sf::Text y50(font, "50%", 9);
    y50.setPosition({bounds.position.x + 5, graphTop + graphH / 2 - 5});
    y50.setFillColor(sf::Color(90, 90, 95));
    window.draw(y50);
    
    sf::Text y0(font, "0%", 9);
    y0.setPosition({bounds.position.x + 8, graphBottom - 12});
    y0.setFillColor(sf::Color(90, 90, 95));
    window.draw(y0);
    
    sf::RectangleShape grid({graphW, 1});
    grid.setPosition({graphX, graphTop + graphH / 2});
    grid.setFillColor(sf::Color(40, 42, 48));
    window.draw(grid);
    
    if (data.size() < 2) return;
    
    float xStep = graphW / (float)(data.size() - 1);
    
    sf::VertexArray lines(sf::PrimitiveType::LineStrip, data.size());
    for (size_t i = 0; i < data.size(); i++) {
        float px = graphX + i * xStep;
        float py = graphTop + (1.0f - data[i]) * graphH;
        lines[i].position = {px, py};
        lines[i].color = color;
    }
    window.draw(lines);
}

void Renderer::drawDNAPanel(sf::RenderWindow& window, Simulation& sim) {
    if (!fontLoaded || sim.selectedPeep < 0 || sim.selectedPeep >= (int)sim.peeps.size()) return;
    
    const Peep& p = sim.peeps[sim.selectedPeep];
    if (!p.alive) return;
    
    float panelW = 380.0f;
    float lineH = 16.0f;
    float panelH = 90.0f + p.genome.size() * lineH * 2 + 20.0f;
    
    float px = worldOffset.x + worldDisplaySize - panelW - 10;
    float py = worldOffset.y + worldDisplaySize - panelH - 10;
    if (py < worldOffset.y + 10) py = worldOffset.y + 10;
    
    sf::RectangleShape bg({panelW, panelH});
    bg.setPosition({px, py});
    bg.setFillColor(sf::Color(15, 17, 22, 240));
    bg.setOutlineColor(sf::Color(60, 65, 75));
    bg.setOutlineThickness(1);
    window.draw(bg);
    
    float ty = py + 10;
    
    sf::Text title(font, "Selected Peep - DNA View", 15);
    title.setPosition({px + 10, ty});
    title.setFillColor(sf::Color(255, 220, 100));
    title.setStyle(sf::Text::Bold);
    window.draw(title);
    ty += 24;
    
    std::ostringstream ss;
    ss << "Hunger: " << std::fixed << std::setprecision(0) << p.hunger
       << "  |  Age: " << std::setprecision(1) << p.age << "s"
       << "  |  Gen: " << p.generation;
    sf::Text stats(font, ss.str(), 12);
    stats.setPosition({px + 10, ty});
    stats.setFillColor(sf::Color(160, 160, 165));
    window.draw(stats);
    ty += 22;
    
    sf::RectangleShape sep({panelW - 20, 1});
    sep.setPosition({px + 10, ty});
    sep.setFillColor(sf::Color(50, 52, 60));
    window.draw(sep);
    ty += 8;
    
    sf::Text hdr(font, "GENOME (32-bit genes):", 12);
    hdr.setPosition({px + 10, ty});
    hdr.setFillColor(sf::Color(140, 140, 145));
    window.draw(hdr);
    
    bool hasMut = false;
    for (size_t i = 0; i < p.mutatedBits.size() && i < p.genome.size(); i++) {
        if (p.mutatedBits[i] != 0) { hasMut = true; break; }
    }
    if (hasMut) {
        sf::Text leg(font, " (mutated bits in red)", 11);
        leg.setPosition({px + 175, ty + 1});
        leg.setFillColor(sf::Color(255, 100, 100));
        window.draw(leg);
    }
    ty += 18;
    
    for (size_t i = 0; i < p.genome.size(); i++) {
        uint32_t gene = p.genome[i];
        uint32_t mask = (i < p.mutatedBits.size()) ? p.mutatedBits[i] : 0;
        
        std::string binary = Gene::toBinaryString(gene);
        
        ss.str("");
        ss << "G" << i << ": ";
        sf::Text num(font, ss.str(), 11);
        num.setPosition({px + 10, ty});
        num.setFillColor(sf::Color(120, 120, 125));
        window.draw(num);
        
        float bx = px + 40;
        float charW = 7.0f;
        
        for (size_t s = 0; s < binary.size(); s++) {
            char ch = binary[s];
            if (ch == ' ') { bx += charW * 0.6f; continue; }
            
            int bit;
            if (s < 4) bit = 31 - (int)s;
            else if (s < 9) bit = 31 - (int)(s - 1);
            else bit = 31 - (int)(s - 2);
            
            bool mut = (mask & (1u << bit)) != 0;
            
            sf::Color col;
            if (bit >= 28) col = sf::Color(100, 200, 220);
            else if (bit >= 24) col = sf::Color(100, 220, 120);
            else col = sf::Color(140, 140, 145);
            if (mut) col = sf::Color(255, 80, 60);
            
            sf::Text bit_text(font, std::string(1, ch), 11);
            bit_text.setPosition({bx, ty});
            bit_text.setFillColor(col);
            if (mut) bit_text.setStyle(sf::Text::Bold);
            window.draw(bit_text);
            
            bx += charW;
        }
        ty += lineH;
        
        std::string readable = Gene::toDisplayString(gene);
        sf::Text rd(font, readable, 11);
        rd.setPosition({px + 40, ty});
        rd.setFillColor(sf::Color(180, 180, 185));
        window.draw(rd);
        ty += lineH + 2;
    }
}

void Renderer::drawEditOverlay(sf::RenderWindow& window, Simulation& sim) {
    if (!fontLoaded) return;
    
    sf::Color fill, outline, prevFill, prevOut, border;
    std::string mode;
    
    if (editingObstacles) {
        fill = sf::Color(80, 60, 60, 180);
        outline = sf::Color(200, 100, 100);
        prevFill = sf::Color(200, 100, 100, 100);
        prevOut = sf::Color(255, 150, 150);
        border = sf::Color(255, 180, 80);
        mode = "OBSTACLES";
    } else if (editingSurvivalZones) {
        fill = sf::Color(60, 100, 60, 180);
        outline = sf::Color(100, 220, 120);
        prevFill = sf::Color(100, 220, 120, 100);
        prevOut = sf::Color(150, 255, 170);
        border = sf::Color(100, 255, 150);
        mode = "SURVIVAL ZONES";
    } else {
        fill = sf::Color(60, 80, 120, 180);
        outline = sf::Color(100, 160, 220);
        prevFill = sf::Color(100, 180, 240, 100);
        prevOut = sf::Color(150, 200, 255);
        border = sf::Color(100, 180, 255);
        mode = "SPAWN ZONES";
    }
    
    sf::RectangleShape dim({worldDisplaySize, worldDisplaySize});
    dim.setPosition(worldOffset);
    dim.setFillColor(sf::Color(0, 0, 0, 40));
    window.draw(dim);
    
    auto drawRects = [&](auto& items) {
        for (auto& item : items) {
            sf::RectangleShape rect(item.size() * worldScale);
            rect.setPosition(worldOffset + item.position() * worldScale);
            rect.setFillColor(fill);
            rect.setOutlineColor(outline);
            rect.setOutlineThickness(2);
            window.draw(rect);
        }
    };
    
    if (editingObstacles) drawRects(sim.obstacles);
    else if (editingSurvivalZones) drawRects(sim.survivalZones);
    else drawRects(sim.spawnZones);
    
    if (isDrawing && !isErasing) {
        sf::Vector2i mpos = sf::Mouse::getPosition(window);
        sf::Vector2f worldEnd = (sf::Vector2f((float)mpos.x, (float)mpos.y) - worldOffset) / worldScale;
        
        float x = std::min(drawStart.x, worldEnd.x);
        float y = std::min(drawStart.y, worldEnd.y);
        float w = std::abs(worldEnd.x - drawStart.x);
        float h = std::abs(worldEnd.y - drawStart.y);
        
        if (w > 5 && h > 5) {
            sf::RectangleShape preview({w * worldScale, h * worldScale});
            preview.setPosition(worldOffset + sf::Vector2f(x, y) * worldScale);
            preview.setFillColor(prevFill);
            preview.setOutlineColor(prevOut);
            preview.setOutlineThickness(2);
            window.draw(preview);
        }
    }
    
    sf::RectangleShape bord({worldDisplaySize, worldDisplaySize});
    bord.setPosition(worldOffset);
    bord.setFillColor(sf::Color::Transparent);
    bord.setOutlineColor(border);
    bord.setOutlineThickness(3);
    window.draw(bord);
    
    std::string instr = "EDITING " + mode + ": Left-drag to draw | Right-click to erase | ESC to exit";
    sf::Text inst(font, instr, 14);
    sf::FloatRect tb = inst.getLocalBounds();
    inst.setPosition({worldOffset.x + (worldDisplaySize - tb.size.x) / 2, worldOffset.y + 10});
    inst.setFillColor(sf::Color(255, 220, 100));
    window.draw(inst);
}
