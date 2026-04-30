#pragma once
#include <string>
#include <vector>
#include <functional>

#include "Marcador.h"
#include "system.h"

//class Marcador; // Forward declare

struct MenuEntry {
    std::string label;
    std::function<void()> onConfirm;
};

class SimpleMenu {
    std::vector<MenuEntry> entries;
    size_t selected = 0;
    const int startY = 32;
    const int lineHeight = 16;

public:
    void clear() { entries.clear(); selected = 0; }
    
    void add(const std::string& text, std::function<void()> action) {
        entries.push_back({text, std::move(action)});
    }

    void handleNavigation() {
        if (entries.empty()) return;
        if (sys->pad.up)   { sys->pad.up = false;   selected--; }
        if (sys->pad.down) { sys->pad.down = false; selected++; }
        
        //int max = static_cast<int>(entries.size()) - 1;
	size_t max = entries.size()-1;
        if (selected > max) selected = 0;
        else if (selected < 0) selected = max;
    }

    bool handleConfirm() {
        if (BUTTON_YES) {
            BUTTON_YES = false;
            if (selected >= 0 && selected < entries.size()) {
                entries[selected].onConfirm();
                return true;
            }
        }
        return false;
    }

    void draw(Abadia::Marcador& marcador) const {
        for (size_t i = 0; i < entries.size(); ++i) {
            int x = (320 - entries[i].label.length() * 8) >> 1;
            bool isSel = (i == selected);
            marcador.imprimeFrase(entries[i].label, x, startY + i * lineHeight,
                                  isSel ? 0 : 4, isSel ? 4 : 0);
        }
    }
};
