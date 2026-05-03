#pragma once
#include <string>
#include <vector>
#include <functional>

#include "system.h"
#include "Marcador.h"

enum class MenuOrientation { VERTICAL, HORIZONTAL };
enum class MenuAlignment   { CENTER, LEFT };
enum class MenuMode { NORMAL, YESNO };

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
    MenuAlignment   alignment   = MenuAlignment::CENTER;
    MenuMode mode = MenuMode::NORMAL;
    std::function<std::string()> getPrompt; 
    const int lineSpacing = 16;
    const int leftMargin  = 80;

public:
    void clear() { entries.clear(); selected = 0; getPrompt = nullptr; mode = MenuMode::NORMAL; }
    void setOrientation(MenuOrientation o) { orientation = o; }
    void setAlignment(MenuAlignment a)     { alignment   = a; }
    void setMode(MenuMode m) { mode = m; }
    void setPrompt(std::function<std::string()> p) { getPrompt = std::move(p); }

    void add(
		    std::function<std::string()> getText,
		    std::function<void()> action,
		    std::function<bool()> enabled = [](){return true;}) {
        entries.emplace_back(std::move(getText), std::move(action), std::move(enabled));
    }

    void fill() {
	    size_t pos = entries.size();
	    while (pos <= 8) {
		    const int n = pos; // captura por valor para el lambda
		    add(
			    [n]() { return std::to_string(n) + " "; },
			    []()  {},
			    []()  { return false; }
		       );
		    pos++;
	    }
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

	// 1.5 Acceso directo por número (1-9, posición absoluta) 
	/*
        {
            //const Uint8 *keys = SDL_GetKeyboardState(nullptr);
            // SDL_SCANCODE_1..9
            for (int n = 1; n <= 9 && n <= (int)entries.size(); ++n) {
                //if (keys[SDL_SCANCODE_0 + n]) {
		if (sys->isNumberKeyPressed(n)) {
                    size_t idx = n - 1;
                    if (entries[idx].isEnabled()) {
                        selected = idx;
                        BUTTON_YES = false;
                        entries[idx].onConfirm();
                        return true;
                    }
                    break;
                }
            }
        } */
	SDL_Log ("lastNumberPressed %d size %d\n",sys->pad.lastNumberPressed ,(int)entries.size());
	// Acceso directo por número (1-9, posición absoluta)
	if (sys->pad.lastNumberPressed >= 0 && 
			sys->pad.lastNumberPressed <= (int)entries.size()-1) {
		//size_t idx = sys->pad.lastNumberPressed - 1;
		size_t idx = sys->pad.lastNumberPressed;
		sys->pad.lastNumberPressed = -1;  // consumir
		if (entries[idx].isEnabled()) {
			selected = idx;
			entries[idx].onConfirm();
			return true;
		}
	}

	// Atajo directo S/N en diálogos binarios
	if (mode == MenuMode::YESNO) {
		if (sys->pad.confirm) {
			sys->pad.confirm = false;
			// buscar la entrada "sí" (índice 0 por convención)
			if (entries[0].isEnabled()) { entries[0].onConfirm(); return true; }
		}
		if (sys->pad.cancel) {
			sys->pad.cancel = false;
			// buscar la entrada "no" (índice 1 por convención)
			if (entries[1].isEnabled()) { entries[1].onConfirm(); return true; }
		}
	}

	// 2. Confirmación
        if (BUTTON_YES) {
//            BUTTON_YES = false;
            if (entries[selected].isEnabled()) {
                entries[selected].onConfirm();
                return true;
            }
        }

        // 3. Dibujado (fondo ya limpio por changeState)
	/*
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
        } */
	// Reemplaza el bloque que empieza por "int y = prompt.empty() ? 48 : 32;"
	//int y = 48;
	int y = 16;
	if (getPrompt) {
		std::string p = getPrompt();
		y = 32;
		size_t nl = p.find('\n');
		std::string l1 = p.substr(0, nl);
		std::string l2 = nl != std::string::npos ? p.substr(nl+1) : "";
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
                size_t x = (alignment == MenuAlignment::LEFT)
                        ? leftMargin
                        : (320 - (int)txt.length() * 8) >> 1;
                // size_t x = (320 - txt.length() * 8) >> 1;
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
