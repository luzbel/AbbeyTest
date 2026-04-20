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

		Count // para poder obtener el tamaño facilmente
	};

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

struct PlayerInput
{
	bool up, down, left, right, button1, button2, button3, button4;
	bool start;
};

#define BUTTON_YES sys->pad.button3
#define BUTTON_NO sys->pad.button2

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

	SDL_Surface *surface;
	SDL_Rect dstrect; // para mantener proporción al escalar
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Window *window;
	SDL_GameController *gamepad;
	SDL_Haptic *hapticDevice;

	std::vector<Mix_Chunk*>sounds;

	uint64_t frameTime=0;
#ifdef __EMSCRIPTEN__
	uint64_t interruptCounter=0;
	uint64_t targetFrameTime=0;
	bool logicInterrupt=false;
#endif

	void initFrame();
	void endFrame();

	void init();
	void quit();
	void stopSound(Abadia::SONIDOS i);
	void playSound(Abadia::SONIDOS i, bool loop=false);
	void updateScreen();
	void handleEvents();
	void hapticFeedback();

	void setFastSpeed();
	void setNormalSpeed();
	Uint32 RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	void updateTexture();
	void exitGame();
	void print(const std::string message);

	void initPaleta(UINT8 *dirPaleta) { _paleta=new Paleta(dirPaleta); }
	void setGamePalette(UINT8 pal) { _paleta->setGamePalette(pal,surface->format); }; // todo, deberia todos ser del tipo setIntro
	void setIntroPalette(void) { _paleta->setGamePalette(5,surface->format); }; // todo, deberia todos ser del tipo setIntro
								    
	void setRGBPixel(UINT32 x, UINT32 y, UINT32 color) {
		assert((x >= 0) && (x < 320));
		assert((y >= 0) && (y < 200));
		assert(color >= 0);
		_pixels[y * _pitch_pixels + x] = color;
	}

	UINT32 getPixel(UINT32 x, UINT32 y) {
		assert((x >= 0) && (x < 320));
		assert((y >= 0) && (y < 200));

		return _pixels[y * _pitch_pixels + x];
	}
								   
	void setPixel(UINT32 x, UINT32 y, UINT8 color) {
		assert((x >= 0) && (x < 320));
		assert((y >= 0) && (y < 200));
		assert((color >= 0) && (color < 256));
		_pixels[y * _pitch_pixels + x] = _paleta->rgb[color];
	};

	void fillMode1Rect(int x, int y, int width, int height, int color) {
		assert((x >= 0) && (x < 320));
		assert((y >= 0) && (y < 200));
		assert((color >= 0) && (color < 256)); 
		assert(((x + width) <= 320) && ((y + height) <= 200));

		fillRect(x, y, width, height, color);
	}
	private:
	void fillRect(int x, int y, int width, int height, int color)
	{
		int xLimit = width + x - 1;

		for (; height > 0; height--, y++){
			if (xLimit < x) {
				std::swap<int>(x, xLimit);
			}

			for (int xx = x; xx <= xLimit; xx++){
				setPixel(xx, y, color);
			}
		}
	};

	UINT32 *_pixels;
	UINT32 _pitch_pixels;
	Paleta *_paleta;	

};
extern System *const sys;

#endif
