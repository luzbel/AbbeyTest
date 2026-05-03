#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <iostream>
#include <cassert>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#ifdef ANDROID
#include <android/log.h>
#endif

#include "SDLPaleta.h"

#ifdef __EMSCRIPTEN__
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 400 
#else
#define WINDOW_WIDTH 1280  
#define WINDOW_HEIGHT 800
#endif

#define TEXTURE_WIDTH 320
#define TEXTURE_HEIGHT 200
#define WINDOW_TITLE "Abbey SDL2 v2.0 build " __DATE__ " " __TIME__
#define GAME_FRAME_TIME 130
#define SCROLL_FRAME_TIME 60

namespace Abadia {
	enum class SONIDOS : UINT8 {
		Abrir = 0,
		Aporrear = 1,
		Campanas = 2,
		Cerrar = 3,
		Coger = 4,
		Dejar = 5,
		Espejo = 6,
		Final = 7,
		Fondo = 8,
		Inicio = 9,
		Pasos = 10,
		Tintineo = 11,

		Count
	};
/*
	enum class STATES: UINT8 {
		INTRO,
		SCROLL,
		MENU,
		LANGUAGE,
		LOAD,
		SAVE,	
		PLAY,
		ASK_NEW_GAME,
		ASK_CONTINUE,
		ASK_EXIT,
		ENDING
	}; 
	*/
	enum class STATES: UINT8 {
		INTRO,
		SCROLL,
		MENU,
		LANGUAGE,
		LOAD,
		SAVE,
		PLAY,
		ASK_NEW_GAME,
		ASK_CONTINUE,
		ASK_EXIT,
		HELP,           // submenú ayuda
		CONFIG,         // submenú configuración
		CONFIG_GFX,     // submenú gráficos
		CONFIG_SND,     // submenú sonido
		ENDING
	};

	constexpr const char* SOUND_FILE_NAMES[] = {
#ifdef ANDROID
	"roms/abadia/abrir.wav",
	"roms/abadia/aporrear.wav",
	"roms/abadia/campanas.wav",
	"roms/abadia/cerrar.wav",
	"roms/abadia/coger.wav",
	"roms/abadia/dejar.wav",
	"roms/abadia/espejo.wav",		
	"roms/abadia/final.wav",		
	"roms/abadia/fondo.wav",		
	"roms/abadia/inicio.wav",		
	"roms/abadia/pasos.wav",
	"roms/abadia/tintineo.wav"
#else
	"./roms/abadia/abrir.wav",
	"./roms/abadia/aporrear.wav",
	"./roms/abadia/campanas.wav",
	"./roms/abadia/cerrar.wav",
	"./roms/abadia/coger.wav",
	"./roms/abadia/dejar.wav",
	"./roms/abadia/espejo.wav",		
	"./roms/abadia/final.wav",		
	"./roms/abadia/fondo.wav",		
	"./roms/abadia/inicio.wav",		
	"./roms/abadia/pasos.wav",
	"./roms/abadia/tintineo.wav"
#endif
	};
} // namespace Abadia

// ----------------------------------------------------------------------------
// PlayerInput: acciones semánticas del juego, independientes del dispositivo.
// handleEvents() mapea teclado y mando a estos campos.
// Los campos booleanos de movimiento/acción siguen siendo "nivel sostenido"
// (true mientras se mantiene pulsado). Los campos de acción puntual
// (confirm, cancel, menu, etc.) se activan en KEYDOWN/BUTTONDOWN y el
// código del juego los pone a false tras consumirlos.
// lastNumberPressed: 0-9 si se pulsó una tecla numérica este frame, -1 si no.
// cameraTarget: 0-7 si se pulsó 1-7 para cambio de cámara durante el juego, -1 si no.
// ----------------------------------------------------------------------------
struct PlayerInput
{
	// --- movimiento (nivel sostenido) ---
	bool up    = false;
	bool down  = false;
	bool left  = false;
	bool right = false;

	// --- acciones en juego (nivel sostenido) ---
	bool action  = false;   // Espacio / Cruz:      coger/dejar objetos, avanzar cinemáticas
	bool actionQ = false;   // Q / L2:              acción espejo izquierda
	bool actionR = false;   // R / R2:              acción espejo derecha

	// --- acciones puntales (consumir tras usar) ---
	bool confirm = false;   // S,Y / Círculo:       sí / confirmar en menú
	bool cancel  = false;   // N   / Cuadrado:      no  / cancelar en menú
	bool menu    = false;   // Escape / Start:      abrir menú
	bool map     = false;   // F5 / Select:         mostrar mapa
	bool save    = false;   // G,W / R1:            grabar partida
	bool load    = false;   // C,L / L1:            cargar partida
	bool toggleGfx = false; // F2 / Triángulo:      cambiar VGA/CPC
	bool toggleFullscreen = false; // F3:           pantalla completa
	bool toggleMute = false;       // M:             silencio
	bool advanceTime = false;      // Enter:         avanzar tiempo (modo info/debug)
	bool cycleCamera = false;      // Tab / L3:      ciclar personaje cámara

	// --- selección directa en menús (1-9), -1 si no se pulsó ---
	int lastNumberPressed = -1;

	// --- cámara directa durante el juego (1-7 → personajes), -1 si no ---
	int cameraTarget = -1;
};

// Compatibilidad con código antiguo que usa BUTTON_YES / BUTTON_NO
#define BUTTON_YES sys->isConfirm()
#define BUTTON_NO  sys->isCancel()

struct System
{	
	PlayerInput pad;
	bool exit = false;
	bool informationMode = false;	
	bool enableJoystick = true;
	bool fullscreen = false;
	bool haveHapticDevice = false;
	int w = WINDOW_WIDTH;
	int h = WINDOW_HEIGHT;
	Uint32 minimumFrameTime = GAME_FRAME_TIME;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const Uint32 rmask = 0xff000000;
	const Uint32 gmask = 0x00ff0000;
	const Uint32 bmask = 0x0000ff00;
	const Uint32 amask = 0x000000ff;
#else
	const Uint32 rmask = 0;
	const Uint32 gmask = 0;
	const Uint32 bmask = 0;
	const Uint32 amask = 0;
#endif

	SDL_Surface  *surface     = nullptr;
	SDL_Rect      dstrect     = {};
	SDL_Renderer *renderer    = nullptr;
	SDL_Texture  *texture     = nullptr;
	SDL_Window   *window      = nullptr;
	SDL_GameController *gamepad      = nullptr;
	SDL_Haptic         *hapticDevice = nullptr;

	std::vector<Mix_Chunk*> sounds;

	uint64_t frameTime = 0;
#ifdef __EMSCRIPTEN__
	uint64_t interruptCounter = 0;
	uint64_t targetFrameTime  = 0;
	bool     logicInterrupt   = false;
#endif
	int currentPalette = 0;

	void initFrame();
	void endFrame();

	void init();
	void quit();
	void stopSound(Abadia::SONIDOS i);
	void playSound(Abadia::SONIDOS i, bool loop = false);
	void pauseSounds();
	void resumeSounds();
	void setMute(bool mute);

	void updateScreen();
	void handleEvents();
	void hapticFeedback();
	bool isConfirm() {
		// pad.confirm es la tecla 's'
		// pero no confirma, es solo para cuando se pide 
		// explicitamente un S o N
		// confirmar es enter o espacio para confirmar 
		// la opción de menu seleccionado o avanzar
		// en la pantalla de intro o en el pergamino
		/*
		if (pad.confirm || pad.advanceTime || pad.action ) {
			pad.confirm = pad.advanceTime = pad.action = false;
			return true;
		} */
		if (pad.advanceTime || pad.action ) {
			pad.advanceTime = pad.action = false;
			return true;
		}
		return false;
	}

	bool isCancel() {
		if (pad.cancel) {
			pad.cancel = false;
			return true;
		}
		return false;
	}

	void setFastSpeed();
	void setNormalSpeed();
	Uint32 RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	void updateTexture();
	void exitGame();
	void print(const std::string message);
	void toggleFullscreenMode();

	void initPaleta(UINT8 *dirPaleta) { _paleta = new Paleta(dirPaleta); }
	void setGamePalette(UINT8 pal)    { currentPalette = pal; _paleta->setGamePalette(pal, surface->format); }
	void setIntroPalette()            { _paleta->setGamePalette(5, surface->format); }
	void resetPalette()               { _paleta->setGamePalette(currentPalette, surface->format); }

	void setRGBPixel(UINT32 x, UINT32 y, UINT32 color) {
		assert(x < 320); assert(y < 200);
		_pixels[y * _pitch_pixels + x] = color;
	}
	UINT32 getPixel(UINT32 x, UINT32 y) {
		assert(x < 320); assert(y < 200);
		return _pixels[y * _pitch_pixels + x];
	}
	void setPixel(UINT32 x, UINT32 y, UINT8 color) {
		assert(x < 320); assert(y < 200); assert(color < 256);
		_pixels[y * _pitch_pixels + x] = _paleta->rgb[color];
	}
	void fillMode1Rect(int x, int y, int width, int height, int color) {
		assert(x >= 0 && x < 320); assert(y >= 0 && y < 200);
		assert(color >= 0 && color < 256);
		assert((x + width) <= 320 && (y + height) <= 200);
		fillRect(x, y, width, height, color);
	}

private:
	void fillRect(int x, int y, int width, int height, int color) {
		int xLimit = width + x - 1;
		for (; height > 0; height--, y++) {
			if (xLimit < x) std::swap(x, xLimit);
			for (int xx = x; xx <= xLimit; xx++)
				setPixel(xx, y, color);
		}
	}

	UINT32  *_pixels       = nullptr;
	UINT32   _pitch_pixels = 0;
	Paleta  *_paleta       = nullptr;
};

extern System *const sys;

#endif
