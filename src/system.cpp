#include <iostream>
#include <vector>
#include "system.h"

static System tmpSys;
System *const sys = &tmpSys;

// ----------------------------------------------------------------------------
// init
// ----------------------------------------------------------------------------

void System::init()
{
	gamepad      = nullptr;
	hapticDevice = nullptr;

	Uint32 windowFlags = SDL_WINDOW_SHOWN;
	Uint32 initFlags   = SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;


	SDL_Init(SDL_INIT_VIDEO | initFlags | SDL_INIT_AUDIO);

#ifdef ANDROID
	windowFlags |= SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE;
#endif
	SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
/*
	if (SDL_NumJoysticks() < 1) {
		print("Warning: controller not found.\n");
	} else {
		gamepad = SDL_GameControllerOpen(0);
		if (!gamepad)
			print("Warning: Can't open SDL_GameControllerOpen(0)\n");
		SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
	}
	*/

#ifndef __EMSCRIPTEN__
	if (SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0) {
		print("Error: Can't init SDL_InitSubSystem(SDL_INIT_HAPTIC).\n");
	} else if (SDL_NumHaptics() > 0) {
		haveHapticDevice = true;
		hapticDevice = SDL_HapticOpen(0);
		if (!hapticDevice) {
			print("Warning: SDL_HapticOpen(0) failed (non-fatal)\n");
		} else if (SDL_HapticRumbleInit(hapticDevice) != 0) {
			print("Warning: SDL_HapticRumbleInit failed (non-fatal)\n");
			SDL_HapticClose(hapticDevice);
			hapticDevice = nullptr;
		}
	}
#endif

#ifdef RG350
	if (SDL_HapticRumblePlay(hapticDevice, 0.30f, 10) < 0)
		print("Warning: SDL_HapticRumbleStop failed\n");
#endif

#ifdef ANDROID
	window = SDL_CreateWindow(WINDOW_TITLE, 0, 0, 0, 0, windowFlags);
	SDL_GetWindowSize(window, &w, &h);
#else
	window = SDL_CreateWindow(WINDOW_TITLE,
	                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          w, h, windowFlags);
	{
		int ww, wh = 0;
		SDL_GetWindowSize(window, &ww, &wh);
		float scale = fminf((float)ww / TEXTURE_WIDTH, (float)wh / TEXTURE_HEIGHT);
		dstrect.w = (int)(TEXTURE_WIDTH  * scale);
		dstrect.h = (int)(TEXTURE_HEIGHT * scale);
		dstrect.x = (ww - dstrect.w) / 2;
		dstrect.y = (wh - dstrect.h) / 2;
	}
#endif

	if (!window)   print("ERROR: Could not create window.\n");

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) print("ERROR: Could not create renderer.\n");

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
		print("ERROR: Mix_OpenAudio\n");
		fprintf(stderr, "ERROR: Mix_OpenAudio %s.\n", SDL_GetError());
	}
	int nc = Mix_AllocateChannels(static_cast<int>(Abadia::SONIDOS::Count));
	if (nc != static_cast<int>(Abadia::SONIDOS::Count))
		fprintf(stderr, "solo %d canales\n", nc);

	for (int i = 0; i < static_cast<int>(Abadia::SONIDOS::Count); i++) {
		fprintf(stderr, "loadwav %s\n", Abadia::SOUND_FILE_NAMES[i]);
		sounds.push_back(Mix_LoadWAV(Abadia::SOUND_FILE_NAMES[i]));
		if (!sounds[i]) print("Error: can't read sound file\n");
	}

	surface = SDL_CreateRGBSurface(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 32,
	                               rmask, gmask, bmask, amask);
	if (!surface) print("Error: Can't create surface.\n");

	_pixels       = static_cast<Uint32 *>(surface->pixels);
	_pitch_pixels = surface->pitch / sizeof(UINT32);

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

#ifdef __EMSCRIPTEN__
	interruptCounter = 0;
	logicInterrupt   = false;
	targetFrameTime  = SDL_GetTicks64();
#endif
}

// ----------------------------------------------------------------------------
// quit
// ----------------------------------------------------------------------------

void System::quit()
{
	SDL_GameControllerClose(gamepad);
	SDL_HapticClose(hapticDevice);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	for (int i = 0; i < static_cast<int>(Abadia::SONIDOS::Count); i++)
		Mix_FreeChunk(sounds[i]);
	sounds.clear();

	SDL_Quit();
}

// ----------------------------------------------------------------------------
// audio
// ----------------------------------------------------------------------------

void System::setMute(bool mute)
{
	Mix_Volume(-1, mute ? 0 : MIX_MAX_VOLUME);
}

void System::pauseSounds()  { Mix_Pause(-1);  }
void System::resumeSounds() { Mix_Resume(-1); }

void System::stopSound(Abadia::SONIDOS index)
{
	int i = static_cast<int>(index);
	assert(i < static_cast<int>(Abadia::SONIDOS::Count));
	Mix_HaltChannel(i);
}

void System::playSound(Abadia::SONIDOS index, bool loop)
{
	int i = static_cast<int>(index);
	assert(i < static_cast<int>(Abadia::SONIDOS::Count));
	Mix_PlayChannel(i, sounds[i], loop ? -1 : 0);
}

// ----------------------------------------------------------------------------
// haptic
// ----------------------------------------------------------------------------

void System::hapticFeedback()
{
	if (!haveHapticDevice) return;
	if (SDL_HapticRumblePlay(hapticDevice, 0.80f, 200) < 0)
		print("Error: SDL_HapticRumblePlay failed\n");
}

// ----------------------------------------------------------------------------
// screen / rendering
// ----------------------------------------------------------------------------

void System::toggleFullscreenMode()
{
	fullscreen = !fullscreen;
	SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

void System::updateScreen()
{
#ifdef __EMSCRIPTEN__
	if (interruptCounter % 6 == 0) {
#endif
	SDL_RenderClear(renderer);
#ifdef ANDROID
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
#else
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderCopy(renderer, texture, nullptr, &dstrect);
#endif
	SDL_RenderPresent(renderer);
#ifdef __EMSCRIPTEN__
	}
#endif
}

void System::updateTexture()
{
	SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
}

// ----------------------------------------------------------------------------
// handleEvents
//
//  Acción           Teclado              Mando
//  ─────────────────────────────────────────────────────
//  Mover            Cursores             Cruceta / analógico izq
//  Confirmar/Sí     S(i), Y(es)          Círculo  (BUTTON_B en SDL)
//  Cancelar/No      N(o)                 Cuadrado (BUTTON_X en SDL)
//  Acción           Espacio              Cruz     (BUTTON_A en SDL)
//  Menú             Escape               Start
//  Mapa             F5,I(nformación)     Select   (BACK)
//  Grabar           G(rabar), W(rite)    R1       (RIGHTSHOULDER)
//  Cargar           C(argar), L(oad)     L1       (LEFTSHOULDER)
//  Cambiar VGA/CPC  F2,V(GA)             Triángulo(BUTTON_Y en SDL)
//  Pantalla completa F3                  —
//  Avanzar tiempo   Enter                —
//  Mute             M(ute)               —
//  Ciclar cámara    Tab                  L3       (LEFTSTICK)
//  Acción Q espejo  Q                    L2       (TRIGGERLEFT via axis)
//  Acción R espejo  R                    R2       (TRIGGERRIGHT via axis)
//  Cámara 1-7       Teclas 1-7           —
//  Selección menú   Teclas 0-9           —
//
// Nota sobre L2/R2: son ejes analógicos en SDL2 (SDL_CONTROLLER_AXIS_TRIGGERLEFT/RIGHT),
// no botones. Se consideran pulsados cuando superan la mitad de su recorrido (> 16383).
// Se procesan en el bloque de eventos de eje (SDL_CONTROLLERAXISMOTION).
//
// Nota sobre teclas numéricas: se registran como eventos discretos (KEYDOWN sin repeat)
// en lastNumberPressed. El código consumidor lo pone a -1 tras usarlo.
// Las teclas 1-7 también se registran en cameraTarget durante el juego.
// ----------------------------------------------------------------------------

void System::handleEvents()
{
	// Resetear campos puntales al inicio de cada llamada.
	// Los campos de nivel sostenido (up/down/left/right/action/actionQ/actionR)
	// se mantienen hasta el KEYUP/BUTTONUP correspondiente.
	pad.confirm           = false;
	pad.cancel            = false;
	pad.menu              = false;
	pad.map               = false;
	pad.save              = false;
	pad.load              = false;
	pad.toggleGfx         = false;
	pad.toggleFullscreen  = false;
	pad.toggleMute        = false;
	pad.advanceTime       = false;
	pad.cycleCamera       = false;
	pad.lastNumberPressed = -1;
	pad.cameraTarget      = -1;

	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		// ----------------------------------------------------------------
		case SDL_QUIT:
			exit = true;
			break;

		// ----------------------------------------------------------------
		case SDL_KEYDOWN:
		{
			// Ignorar repeticiones de tecla mantenida para acciones puntales
			const bool repeat = event.key.repeat != 0;

			switch (event.key.keysym.sym)
			{
			// Movimiento (nivel sostenido)
			case SDLK_UP:    pad.up    = true; break;
			case SDLK_DOWN:  pad.down  = true; break;
			case SDLK_LEFT:  pad.left  = true; break;
			case SDLK_RIGHT: pad.right = true; break;

			// Acción / coger / avanzar cinemática (nivel sostenido)
			case SDLK_SPACE: pad.action = true; break;

			// Acciones espejo (nivel sostenido)
			case SDLK_q: pad.actionQ = true; break;
			case SDLK_r: pad.actionR = true; break;

			// Confirmar (puntual, sin repeat)
			case SDLK_s:
			case SDLK_y:
				if (!repeat) pad.confirm = true;
				break;

			// Cancelar (puntual, sin repeat)
			case SDLK_n:
				if (!repeat) pad.cancel = true;
				break;

			// Menú (puntual, sin repeat)
			case SDLK_ESCAPE:
				if (!repeat) pad.menu = true;
				break;

			// Mapa (puntual, sin repeat)
			case SDLK_F5:
			case SDLK_i:
				if (!repeat) pad.map = true;
				break;

			// Grabar (puntual, sin repeat)
			case SDLK_g:
			case SDLK_w:
				if (!repeat) pad.save = true;
				break;

			// Cargar (puntual, sin repeat)
			case SDLK_c:
			case SDLK_l:
				if (!repeat) pad.load = true;
				break;

			// Cambiar VGA/CPC (puntual, sin repeat)
			case SDLK_v:
			case SDLK_F2:
				if (!repeat) pad.toggleGfx = true;
				break;

			// Pantalla completa (puntual, sin repeat)
			case SDLK_F3:
				if (!repeat) pad.toggleFullscreen = true;
				break;

			// Mute (puntual, sin repeat)
			case SDLK_m:
				if (!repeat) pad.toggleMute = true;
				break;

			// Avanzar tiempo en modo información (puntual, sin repeat)
			case SDLK_RETURN:
			case SDLK_KP_ENTER:
				if (!repeat) pad.advanceTime = true;
				break;

			// Ciclar cámara (puntual, sin repeat)
			case SDLK_TAB:
				if (!repeat) pad.cycleCamera = true;
				break;

			// Teclas numéricas 0-9: selección directa en menús y cambio de cámara
			case SDLK_0:
			case SDLK_1: case SDLK_2: case SDLK_3:
			case SDLK_4: case SDLK_5: case SDLK_6:
			case SDLK_7: case SDLK_8: case SDLK_9:
				if (!repeat) {
					//int n = event.key.keysym.sym - SDLK_1 + 1; // 1..9
					//int n = event.key.keysym.sym - SDLK_0 + 1; // 1..9
					int n = event.key.keysym.sym - SDLK_0; // 1..9
					pad.lastNumberPressed = n;
					if (n >= 1 && n <= 7)
						pad.cameraTarget = n - 1; // índice 0..6 de personajes
				}
				break;

			default:
				break;
			}
			break; // SDL_KEYDOWN
		}

		// ----------------------------------------------------------------
		case SDL_KEYUP:
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:    pad.up     = false; break;
			case SDLK_DOWN:  pad.down   = false; break;
			case SDLK_LEFT:  pad.left   = false; break;
			case SDLK_RIGHT: pad.right  = false; break;
			case SDLK_SPACE: pad.action = false; break;
			case SDLK_q:     pad.actionQ = false; break;
			case SDLK_r:     pad.actionR = false; break;
			default: break;
			}
			break; // SDL_KEYUP

		// hotplug para los mandos
		case SDL_CONTROLLERDEVICEADDED:
			if (!gamepad) { // Si no tenemos ya uno abierto
				gamepad = SDL_GameControllerOpen(event.cdevice.which);
				print("Gamepad conectado!\n");
			}
			break;

		case SDL_CONTROLLERDEVICEREMOVED:
			// event.cdevice.which es el ID de instancia del mando que se fue
			if (gamepad && event.cdevice.which == 
					SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad))) {
				SDL_GameControllerClose(gamepad);
				gamepad = NULL;
				print("Gamepad desconectado.\n");
			}
			break;

		// ----------------------------------------------------------------
		case SDL_CONTROLLERBUTTONDOWN:
		{
			const bool repeat = false; // los mandos no tienen repeat
			switch (event.cbutton.button)
			{
			case SDL_CONTROLLER_BUTTON_DPAD_UP:    pad.up    = true; break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  pad.down  = true; break;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  pad.left  = true; break;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: pad.right = true; break;

			// Cruz → Acción
			case SDL_CONTROLLER_BUTTON_A: pad.action  = true; break;
			// Círculo → Confirmar
			case SDL_CONTROLLER_BUTTON_B: pad.confirm = true; break;
			// Cuadrado → Cancelar
			case SDL_CONTROLLER_BUTTON_X: pad.cancel  = true; break;
			// Triángulo → Cambiar VGA/CPC
			case SDL_CONTROLLER_BUTTON_Y: pad.toggleGfx = true; break;

			// Start → Menú
			case SDL_CONTROLLER_BUTTON_START: pad.menu = true; break;
			// Select/Back → Mapa
			case SDL_CONTROLLER_BUTTON_BACK:  pad.map  = true; break;

			// L1 → Cargar
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  pad.load = true; break;
			// R1 → Grabar
			case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: pad.save = true; break;

			// L3 → Ciclar cámara
			case SDL_CONTROLLER_BUTTON_LEFTSTICK: pad.cycleCamera = true; break;

			default: break;
			}
			(void)repeat;
			break; // SDL_CONTROLLERBUTTONDOWN
		}

		// ----------------------------------------------------------------
		case SDL_CONTROLLERBUTTONUP:
			switch (event.cbutton.button)
			{
			case SDL_CONTROLLER_BUTTON_DPAD_UP:    pad.up     = false; break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  pad.down   = false; break;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  pad.left   = false; break;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: pad.right  = false; break;
			case SDL_CONTROLLER_BUTTON_A:          pad.action = false; break;
			default: break;
			}
			break; // SDL_CONTROLLERBUTTONUP

		// ----------------------------------------------------------------
		// L2 / R2 son ejes analógicos en SDL2, no botones.
		// Umbral: mitad del rango positivo (32767 / 2 ≈ 16383).
		case SDL_CONTROLLERAXISMOTION:
			switch (event.caxis.axis)
			{
			case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
				pad.actionQ = (event.caxis.value > 16383);
				break;
			case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
				pad.actionR = (event.caxis.value > 16383);
				break;
			// Analógico izquierdo para movimiento
			case SDL_CONTROLLER_AXIS_LEFTX:
				if (event.caxis.value < -8192)       pad.left  = true;
				else if (event.caxis.value > 8192)    pad.right = true;
				else { pad.left = false; pad.right = false; }
				break;
			case SDL_CONTROLLER_AXIS_LEFTY:
				if (event.caxis.value < -8192)        pad.up   = true;
				else if (event.caxis.value > 8192)     pad.down = true;
				else { pad.up = false; pad.down = false; }
				break;
			default: break;
			}
			break; // SDL_CONTROLLERAXISMOTION

		// ----------------------------------------------------------------
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED ||
			    event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				int ww, wh = 0;
				SDL_GetWindowSize(window, &ww, &wh);
				float scale = fminf((float)ww / TEXTURE_WIDTH, (float)wh / TEXTURE_HEIGHT);
				dstrect.w = (int)(TEXTURE_WIDTH  * scale);
				dstrect.h = (int)(TEXTURE_HEIGHT * scale);
				dstrect.x = (ww - dstrect.w) / 2;
				dstrect.y = (wh - dstrect.h) / 2;
			}
			break;

		default:
			break;
		} // switch event.type
	} // while PollEvent
}

// ----------------------------------------------------------------------------
// velocidad
// ----------------------------------------------------------------------------

void System::setFastSpeed()   { minimumFrameTime = SCROLL_FRAME_TIME; }
void System::setNormalSpeed() { minimumFrameTime = GAME_FRAME_TIME;   }

// ----------------------------------------------------------------------------
// utilidades
// ----------------------------------------------------------------------------

Uint32 System::RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return SDL_MapRGBA(surface->format, r, g, b, a);
}

void System::exitGame() { exit = true; }

void System::print(const std::string message)
{
#ifdef ANDROID
	__android_log_print(ANDROID_LOG_DEBUG, "ABBEY", "%s\n", message.c_str());
#else
	std::cout << message;
#endif
}

// ----------------------------------------------------------------------------
// frame timing
// ----------------------------------------------------------------------------

void System::initFrame()
{
	frameTime = SDL_GetTicks64();
#ifdef __EMSCRIPTEN__
	interruptCounter++;
	if (frameTime >= targetFrameTime) logicInterrupt = true;
#endif
}

void System::endFrame()
{
#ifndef __EMSCRIPTEN__
	uint64_t elapsed = SDL_GetTicks64() - frameTime;
	if (elapsed < minimumFrameTime)
		SDL_Delay(minimumFrameTime - elapsed);
#else
	if (logicInterrupt) {
		targetFrameTime += 0x24 * (1000. / 300.);
		if (targetFrameTime <= frameTime)
			targetFrameTime = SDL_GetTicks64() + 5;
		logicInterrupt = false;

		static auto lastLogic  = SDL_GetTicks64();
		static int  framesLogic = 0;
		framesLogic++;
		if (SDL_GetTicks64() - lastLogic > 1000) {
			framesLogic = 0;
			lastLogic   = SDL_GetTicks64();
		}
	}
#endif

	static auto last   = SDL_GetTicks64();
	static int  frames = 0;
	frames++;
	if (SDL_GetTicks64() - last > 1000) {
		frames = 0;
		last   = SDL_GetTicks64();
	}
}
