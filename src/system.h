#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <functional>
#include <ctime>
#include <cmath>        // fminf

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#ifdef ANDROID
#include <android/log.h>
#endif

// --- OpenGL ---
#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#include "SDLPaleta.h"
#include "configreader.h"

#ifdef __EMSCRIPTEN__
#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 400
#else
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 800
#endif

#define TEXTURE_WIDTH  320
#define TEXTURE_HEIGHT 200
#define WINDOW_TITLE   "Abbey SDL2 v2.0 build " __DATE__ " " __TIME__
#define GAME_FRAME_TIME   130
#define SCROLL_FRAME_TIME  60

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

	enum class STATES : UINT8 {
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
		HELP,
		HELP_INTRODUCCION,
		HELP_MANEJO_PERGAMINO,
		HELP_MANEJO,
		HELP_AYUDAS,
		HELP_CAMARAS,
		HELP_REFERENCIAS,
		CONFIG,
		CONFIG_GFX,
		CONFIG_SND,
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
// ConfigVar<T>
// ----------------------------------------------------------------------------
template<typename T>
class ConfigVar {
public:
	ConfigVar(const std::string& key, T defaultValue)
		: _key(key), _value(defaultValue), _configReader(nullptr) {}

	void bind(ConfigReader* cr) { _configReader = cr; }

	void setCallback(std::function<void(const T&)> cb) { _onSet = std::move(cb); }

	void load() {
		SDL_Log("ConfigVar load\n");
		if (!_configReader) return;
		SDL_Log("ConfigVar load hay _configReader\n");
		const std::string s = _configReader->getValue(_key);
		SDL_Log("ConfigVar load %s = *%s*\n",_key.c_str(),s.c_str());
		if (!s.empty()) _value = fromString(s);
		SDL_Log("ConfigVar\n");
		if (_onSet) _onSet(_value);
		SDL_Log("ConfigVar despues de _onSet\n");
	}

	ConfigVar& operator=(const T& v) {
		_value = v;
		persist();
		if (_onSet) _onSet(_value);
		return *this;
	}

	operator T()  const { return _value; }
	T get()       const { return _value; }
	const std::string& key() const { return _key; }

private:
	std::string   _key;
	T             _value;
	ConfigReader* _configReader;

	void persist() {
		if (_configReader) _configReader->setValue(_key, toString(_value));
	}

	static std::string toString(const T& v);
	static T fromString(const std::string& s);

	std::function<void(const T&)> _onSet;
};

template<> inline std::string ConfigVar<bool>::toString(const bool& v)         { return v ? "1" : "0"; }
template<> inline bool        ConfigVar<bool>::fromString(const std::string& s) { return s == "1"; }
template<> inline std::string ConfigVar<int>::toString(const int& v)            { return std::to_string(v); }
template<> inline int         ConfigVar<int>::fromString(const std::string& s)  { return std::atoi(s.c_str()); }

// ----------------------------------------------------------------------------
// PlayerInput
// ----------------------------------------------------------------------------
struct PlayerInput
{
	bool up    = false;
	bool down  = false;
	bool left  = false;
	bool right = false;

	bool action  = false;
	bool actionQ = false;
	bool actionR = false;

	bool confirm          = false;
	bool cancel           = false;
	bool menu             = false;
	bool map              = false;
	bool save             = false;
	bool load             = false;
	bool toggleGfx        = false;
	bool toggleFullscreen = false;
	bool toggleMute       = false;
	bool advanceTime      = false;
	bool cycleCamera      = false;

	int lastNumberPressed = -1;
	int cameraTarget      = -1;
};

#define BUTTON_YES sys->isConfirm()
#define BUTTON_NO  sys->isCancel()

// ----------------------------------------------------------------------------
// System
// ----------------------------------------------------------------------------
struct System
{
	PlayerInput pad;
	bool exit             = false;
	bool informationMode  = false;
	bool enableJoystick   = true;
	bool fullscreen       = false;
	bool haveHapticDevice = false;
	int  w = WINDOW_WIDTH;
	int  h = WINDOW_HEIGHT;

	// --- Variables de configuración persistentes ---
	ConfigVar<bool> mute        {"MUTESOUND",   false};
	ConfigVar<bool> GraficosCPC {"GRAPHICSCPC", false};
	ConfigVar<int>  idioma      {"LANGUAGE",    1};
	ConfigVar<int>  filtro      {"FILTER",      0};     // 0=off 1=xbr 2=hqx
	ConfigVar<bool> scanlines   {"SCANLINES",   false};
	ConfigVar<bool> useWebGL    {"USEWEBGL",    true};
	ConfigVar<int>  paletaEfecto{"PALETA",      0};     // 0=normal 1=grises 2=verde 3=ambar

	// --- Slots de guardado ---
	static const int NUM_SLOTS = 7;
	std::string slotDates[NUM_SLOTS];

	// --- Máscaras RGBA correctas para SDL_CreateRGBSurface ---
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const Uint32 rmask = 0xff000000;
	const Uint32 gmask = 0x00ff0000;
	const Uint32 bmask = 0x0000ff00;
	const Uint32 amask = 0x000000ff;
#else
	const Uint32 rmask = 0x000000ff;
	const Uint32 gmask = 0x0000ff00;
	const Uint32 bmask = 0x00ff0000;
	const Uint32 amask = 0xff000000;
#endif

	SDL_Surface        *surface      = nullptr;
	SDL_Surface        *surfaceMap   = nullptr;
	SDL_Surface        *surfaceMenu  = nullptr;
	SDL_Surface        *surfaceIntro = nullptr;
	SDL_Rect            dstrect      = {};
	SDL_Renderer       *renderer     = nullptr;
	SDL_Texture        *texture      = nullptr;
	SDL_Texture        *textureMap   = nullptr;
	SDL_Texture        *textureMenu  = nullptr;
	SDL_Texture        *textureIntro = nullptr;
	SDL_Window         *window       = nullptr;
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

	// --- Ciclo de vida ---
	void init();
	void quit();
	void initFrame();
	void endFrame();

	// --- Config ---
	void               loadConfig();
	void               saveConfig();
	static const char* configPath();

	// --- Slots de guardado ---
	void loadSlotDates();
	bool saveSlot(int slot, std::function<void(std::ofstream&)> writer);
	bool loadSlot(int slot, std::function<void(std::ifstream&)> reader);

	// --- Utilidades ---
	std::string getDateAndTime();

	// --- Audio ---
	void stopSound(Abadia::SONIDOS i);
	void playSound(Abadia::SONIDOS i, bool loop = false);
	void pauseSounds();
	void resumeSounds();
	void setMute(bool m);

	// --- Pantalla ---
	void   updateScreen();
//	void   updateTexture();
	void   toggleFullscreenMode();
	void   handleEvents();
	void   hapticFeedback();
	Uint32 RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	void   exitGame();
	void   print(const std::string message);
	void   setFastSpeed();
	void   setNormalSpeed();

	bool isConfirm() {
		if (pad.advanceTime || pad.action) {
			pad.advanceTime = pad.action = false;
			return true;
		}
		return false;
	}
	bool isCancel() {
		if (pad.cancel) { pad.cancel = false; return true; }
		return false;
	}

	// --- Paleta ---
	// En modo GL el efecto de paleta lo hace el shader; en SW lo hace Paleta.
	// setGamePalette siempre actualiza la paleta SW (se usa para indexar pixels).
	// El shader recoge paletaEfecto en cada frame via uniform.
	void initPaleta(UINT8 *dirPaleta) { _paleta = new Paleta(dirPaleta); }

	void setGamePalette(UINT8 pal) {
		currentPalette = pal;
		// En modo GL no aplicamos efecto SW (lo hace el shader)
//		bool aplicarEfectoSW = !(bool)useWebGL;
		bool useShader=(bool)useWebGL;
		_paleta->setGamePalette(pal, surface->format, useShader, paletaEfecto);
	}

	void setIntroPalette() {
		//bool aplicarEfectoSW = !(bool)useWebGL;
		bool useShader=(bool)useWebGL;
		_paleta->setGamePalette(5, surface->format, useShader, paletaEfecto);
	}

	void resetPalette() {
		if (!_paleta) return;
		//bool aplicarEfectoSW = !(bool)useWebGL;
		bool useShader=(bool)useWebGL;
		_paleta->setGamePalette(currentPalette, surface->format, useShader, paletaEfecto);
	}

	// --- Pixels ---
	void setRGBPixel(UINT32 x, UINT32 y, UINT32 color) {
		assert(x < 320); assert(y < 200);
		_pixels[y * _pitch_pixels + x] = color;
	}
	UINT32 getPixel(UINT32 x, UINT32 y) {
		assert(x < 320); assert(y < 200);
		return _pixels[y * _pitch_pixels + x];
	}
	void setPixelIntro(UINT32 x, UINT32 y, UINT8 color) {
		assert(x < 320); assert(y < 200); assert(color < 256);
		if ((bool)useWebGL)
			_pixelsIntro[y * _pitch_pixels + x] = _paleta->rgb[color];
		else
			setPixel(x,y,color);
	}
	void setPixelMap(UINT32 x, UINT32 y, UINT8 color) {
		assert(x < 320); assert(y < 200); assert(color < 256);
		if (useWebGL)
			_pixelsMap[y * _pitch_pixels + x] = _paleta->rgb[color];
		else
			setPixel(x,y,color);
	}
	void setPixelMenu(UINT32 x, UINT32 y, UINT8 color) {
		assert(x < 320); assert(y < 200); assert(color < 256);
		if ((bool)useWebGL)
			_pixelsMenu[y * _pitch_pixels + x] = _paleta->rgb[color];
		else
			setPixel(x,y,color); 
	}
	void limpiaMenu(void) {
		SDL_FillRect(surfaceMenu, nullptr, _paleta->rgb[4]);
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
	Abadia::STATES _state; // para saber que textura actualizar
	float uFlipT=1.0f;

private:
	void fillRect(int x, int y, int width, int height, int color) {
		int xLimit = width + x - 1;
		for (; height > 0; height--, y++) {
			if (xLimit < x) std::swap(x, xLimit);
			for (int xx = x; xx <= xLimit; xx++)
				setPixel(xx, y, color);
		}
	}

	void initShader(int efectoPaleta);

	std::string slotPath(int slot);

	UINT32       *_pixels          = nullptr;
	UINT32       *_pixelsMenu      = nullptr;
	UINT32       *_pixelsMap       = nullptr;
	UINT32       *_pixelsIntro     = nullptr;
	UINT32        _pitch_pixels    = 0;
	Paleta       *_paleta          = nullptr;
	Uint32        minimumFrameTime = GAME_FRAME_TIME;
	ConfigReader *_configReader    = nullptr;
};

extern System *const sys;

#endif // SYSTEM_H
