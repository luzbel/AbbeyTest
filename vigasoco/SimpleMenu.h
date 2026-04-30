#pragma once
#include <string>
#include <vector>
#include <functional>

#include "Marcador.h"
#include "system.h"

struct MenuEntry {
    std::function<std::string()> getLabel;      // Se evalúa en cada frame
    std::function<void()> onConfirm;
    std::function<bool()> isEnabled = [](){ return true; }; // Opcional: para items grises

    // Constructor explícito: resuelve el error de deducción con initializer lists
    MenuEntry(std::function<std::string()> label,
		    std::function<void()> confirm,
		    std::function<bool()> enabled = [](){ return true; })
	    : getLabel(std::move(label)),
	    onConfirm(std::move(confirm)),
	    isEnabled(std::move(enabled)) {}
};

class SimpleMenu {
    std::vector<MenuEntry> entries;
    size_t selected = 0;
    const int startY = 32;
    const int lineHeight = 16;

public:
    void clear() { entries.clear(); selected = 0; }
    
    // Acepta un callback que devuelve el texto actual
    void add(std::function<std::string()> getText, 
             std::function<void()> action, 
             std::function<bool()> enabled = [](){ return true; }) {
        entries.emplace_back(std::move(getText), std::move(action), std::move(enabled));
    }
/*
    void handleNavigation() {
        if (entries.empty()) return;
        if (sys->pad.up)   { sys->pad.up = false;   selected--; }
        if (sys->pad.down) { sys->pad.down = false; selected++; }
        
        size_t max = entries.size() - 1;
        if (selected > max) selected = 0;
        else if (selected < 0) selected = max;

        // Saltar items deshabilitados al navegar
        while (!entries[selected].isEnabled()) {
            selected = (selected + 1) % entries.size();
            // Evita bucle infinito si todos están deshabilitados
            bool allDisabled = true;
            for (const auto& e : entries) if (e.isEnabled()) { allDisabled = false; break; }
            if (allDisabled) break;
        }
    } 
    */
    void handleNavigation() {
        if (entries.empty()) return;
        int dir = 0;
        if (sys->pad.up)   { sys->pad.up = false; dir = -1; }
        if (sys->pad.down) { sys->pad.down = false; dir = 1; }

        if (dir != 0) {
            int next = selected;
            int count = static_cast<int>(entries.size());

            // Recorre como máximo 'count' posiciones en la dirección elegida
            for (int i = 0; i < count; ++i) {
                next += dir;
                if (next >= count) next = 0;
                else if (next < 0) next = count - 1;

                if (entries[next].isEnabled()) {
                    selected = next;
                    return; // Encontrado, salir
                }
            }
            // Si todas las opciones están desactivadas, se queda en la última evaluada
            // (evita bucles infinitos y mantiene el cursor estable)
        }
    }

    bool handleConfirm() {
        if (BUTTON_YES) {
            BUTTON_YES = false;
            if (selected >= 0 && selected < entries.size() && entries[selected].isEnabled()) {
                entries[selected].onConfirm();
                return true;
            }
        }
        return false;
    }

    void draw(Abadia::Marcador& marcador) const {
        for (size_t i = 0; i < entries.size(); ++i) {
            std::string txt = entries[i].getLabel();
            int x = (320 - txt.length() * 8) >> 1;
            bool isSel = (i == selected);
            bool enabled = entries[i].isEnabled();
            
            // Color 0=fondo/4=texto (normal), 5=gris (deshabilitado)
            int fg = isSel ? 0 : (enabled ? 4 : 5);
            int bg = isSel ? 4 : 0;
            
            marcador.imprimeFrase(txt, x, startY + i * lineHeight, fg, bg);
        }
    }
};
