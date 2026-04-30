#pragma once
#include <string>
#include <vector>
#include <functional>

#include "system.h"
#include "Marcador.h"

enum class MenuOrientation { VERTICAL, HORIZONTAL };

struct MenuEntry {
    std::function<std::string()> getLabel;
    std::function<void()> onConfirm;
    std::function<bool()> isEnabled;
    MenuEntry(std::function<std::string()> l, std::function<void()> a, std::function<bool()> e = [](){return true;})
        : getLabel(std::move(l)), onConfirm(std::move(a)), isEnabled(std::move(e)) {}
};

class SimpleMenu {
    std::vector<MenuEntry> entries;
    size_t selected = 0;
    MenuOrientation orientation = MenuOrientation::VERTICAL;
    std::string prompt;
    const int lineSpacing = 16;

public:
    void clear() { entries.clear(); selected = 0; prompt.clear(); }
    void setOrientation(MenuOrientation o) { orientation = o; }
    void setPrompt(const std::string& p) { prompt = p; }

    void add(std::function<std::string()> getText, std::function<void()> action, std::function<bool()> enabled = [](){return true;}) {
        entries.emplace_back(std::move(getText), std::move(action), std::move(enabled));
    }

    // ÚNICO PUNTO DE EJECUCIÓN: Input + Wrap + Confirm + Draw. Devuelve true al seleccionar.
    bool tick(Abadia::Marcador& marcador) {
        if (entries.empty()) return false;

        // 1. Navegación
        int dir = 0;
        if (orientation == MenuOrientation::VERTICAL) {
            if (sys->pad.up)   { dir = -1; sys->pad.up = false; }
            if (sys->pad.down) { dir =  1; sys->pad.down = false; }
        } else {
            if (sys->pad.left)  { dir = -1; sys->pad.left = false; }
            if (sys->pad.right) { dir =  1; sys->pad.right = false; }
        }

        if (dir != 0) {
            int next = selected + dir;
            int count = entries.size();
            for (int i = 0; i < count; ++i) {
                if (next >= count) next = 0;
                else if (next < 0) next = count - 1;
                if (entries[next].isEnabled()) { selected = next; break; }
                next += dir;
            }
        }

        // 2. Confirmación
        if (BUTTON_YES) {
            BUTTON_YES = false;
            if (entries[selected].isEnabled()) {
                entries[selected].onConfirm();
                return true;
            }
        }

        // 3. Dibujado (fondo ya limpio por changeState)
        int y = prompt.empty() ? 48 : 32;
        if (!prompt.empty()) {
            size_t nl = prompt.find('\n');
            std::string l1 = prompt.substr(0, nl);
            std::string l2 = nl != std::string::npos ? prompt.substr(nl+1) : "";
            int x = (320 - l1.length() * 8) >> 1;
            marcador.imprimeFrase(l1, x, y, 4, 0);
            if (!l2.empty()) {
                x = (320 - l2.length() * 8) >> 1;
                marcador.imprimeFrase(l2, x, y + 12, 4, 0);
                y += 24;
            } else y += 20;
        }

        if (orientation == MenuOrientation::VERTICAL) {
            for (size_t i = 0; i < entries.size(); ++i) {
                std::string txt = entries[i].getLabel();
                size_t x = (320 - txt.length() * 8) >> 1;
                bool isSel = (i == selected);
                bool enabled = entries[i].isEnabled();
                marcador.imprimeFrase(txt, x, y + i * lineSpacing, isSel ? 0 : (enabled ? 4 : 5), isSel ? 4 : 0);
            }
        } else {
            int totalW = 0;
            for (auto& e : entries) totalW += e.getLabel().length() * 8 + 40;
            int xCursor = (320 - totalW) >> 1;
            for (size_t i = 0; i < entries.size(); ++i) {
                std::string txt = entries[i].getLabel();
                bool isSel = (i == selected);
                bool enabled = entries[i].isEnabled();
                marcador.imprimeFrase(txt, xCursor, y, isSel ? 0 : (enabled ? 4 : 5), isSel ? 4 : 0);
                xCursor += txt.length() * 8 + 40;
            }
        }
        return false;
    }
    bool isEmpty() const { return entries.empty(); }
};
