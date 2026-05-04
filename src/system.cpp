#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
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

	 // Vincular efectos secundarios a variables de configuración
	mute.setCallback([this](bool v) { setMute(v); });
	//paletaEfecto.setCallback([this](int) { resetPalette(); }); //no hace falta???

	// Cargar configuración — debe ser lo último de init para que
	// surface y renderer ya existan si algún efecto los necesita.
	loadConfig();
}

// ----------------------------------------------------------------------------
// quit
// ----------------------------------------------------------------------------

void System::quit()
{
	saveConfig();

	SDL_GameControllerClose(gamepad);
	SDL_HapticClose(hapticDevice);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	for (int i = 0; i < static_cast<int>(Abadia::SONIDOS::Count); i++)
		Mix_FreeChunk(sounds[i]);
	sounds.clear();

	delete _configReader;
	_configReader = nullptr;

	SDL_Quit();
}

// ----------------------------------------------------------------------------
// Config
// ----------------------------------------------------------------------------

const char* System::configPath()
{
#if defined(RG350)
	return "/usr/local/home/Abbey/config.txt";
#elif defined(__EMSCRIPTEN__)
	return "/save/config.txt";
#elif defined(VITA)
	return "ux0:data/Abbey/config.txt";
#elif defined(ANDROID)
	// En Android la ruta depende del storage externo; se resuelve en runtime.
	// Devolvemos un fallback; el código Android puede sobreescribir si necesita.
	return "config.txt";
#else
	return "config.txt";
#endif
}

void System::loadConfig()
{
	delete _configReader;
	_configReader = new ConfigReader(configPath());

	mute        .bind(_configReader);
	GraficosCPC .bind(_configReader);
	idioma      .bind(_configReader);
	filtro      .bind(_configReader);
	scanlines   .bind(_configReader);
	useWebGL    .bind(_configReader);
	paletaEfecto.bind(_configReader);

	if (_configReader->parse()) {
		mute        .load();
		GraficosCPC .load();
		idioma      .load();
		filtro      .load();
		scanlines   .load();
		useWebGL    .load();
		paletaEfecto.load();
	}

	// Efecto secundario inmediato tras cargar
	//setMute(mute);
	//ya va automático con lambdas al cargar o al asignar

	loadSlotDates();
}

void System::saveConfig()
{
	if (!_configReader) return;

	// Las ConfigVar ya actualizaron sus valores en el reader al asignarse.
	// Solo volcamos el reader a disco.
	std::ofstream f(configPath());
	for (auto& [key, value] : _configReader->getData())
		f << key << "=" << value << "\n";
}

// ----------------------------------------------------------------------------
// Slots de guardado
// ----------------------------------------------------------------------------

std::string System::slotPath(int slot)
{
	std::string path;
#if defined(RG350)
	path = "/usr/local/home/Abbey/";
#elif defined(__EMSCRIPTEN__)
	path = "/save/";
#elif defined(VITA)
	path = "ux0:data/Abbey/";
#elif defined(ANDROID)
	if (SDL_AndroidGetExternalStorageState() != 0) {
		path = SDL_AndroidGetExternalStoragePath();
		path += "/";
	}
#endif
	path += "abadia";
	path += static_cast<char>('0' + slot);
	path += ".save";
	return path;
}

void System::loadSlotDates()
{
	if (!_configReader) return;
	for (int i = 0; i < NUM_SLOTS; i++) {
		std::string token = "SAVEX";
		token[4] = static_cast<char>('0' + i);
		slotDates[i] = _configReader->getValue(token);
		if (slotDates[i].empty()) slotDates[i] = "--";
	}
}

bool System::saveSlot(int slot, std::function<void(std::ofstream&)> writer)
{
	assert(slot >= 0 && slot < NUM_SLOTS);

	std::ofstream out(slotPath(slot), std::ofstream::out | std::ofstream::trunc);
	if (!out) {
		print("Error: no se puede abrir el fichero de guardado.\n");
		return false;
	}
	writer(out);
	if (out.fail()) {
		print("Error: fallo al escribir el fichero de guardado.\n");
		return false;
	}

	// Actualizar fecha en config
	std::string token = "SAVEX";
	token[4] = static_cast<char>('0' + slot);
	const std::string fecha = getDateAndTime();
	if (_configReader) _configReader->setValue(token, fecha);
	slotDates[slot] = fecha;
	saveConfig();

	return true;
}

bool System::loadSlot(int slot, std::function<void(std::ifstream&)> reader)
{
	assert(slot >= 0 && slot < NUM_SLOTS);

	std::ifstream in(slotPath(slot));
	if (!in) {
		print("Error: no se puede abrir el fichero de guardado.\n");
		return false;
	}
	reader(in);
	return !in.fail();
}

// ----------------------------------------------------------------------------
// getDateAndTime
// ----------------------------------------------------------------------------

std::string System::getDateAndTime()
{
	time_t t = time(nullptr);
	struct tm tm = *localtime(&t);
	char buff[32];
	snprintf(buff, sizeof(buff), "%d.%02d.%02d-%02d:%02d",
	         tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900,
	         tm.tm_hour, tm.tm_min);
	return buff;
}

// ----------------------------------------------------------------------------
// audio
// ----------------------------------------------------------------------------

void System::setMute(bool m)
{
	Mix_Volume(-1, m ? 0 : MIX_MAX_VOLUME);
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
// ----------------------------------------------------------------------------

void System::handleEvents()
{
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
		case SDL_QUIT:
			exit = true;
			break;

		case SDL_KEYDOWN:
		{
			const bool repeat = event.key.repeat != 0;
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:    pad.up    = true; break;
			case SDLK_DOWN:  pad.down  = true; break;
			case SDLK_LEFT:  pad.left  = true; break;
			case SDLK_RIGHT: pad.right = true; break;
			case SDLK_SPACE: pad.action = true; break;
			case SDLK_q:     pad.actionQ = true; break;
			case SDLK_r:     pad.actionR = true; break;
			case SDLK_s:
			case SDLK_y:
				if (!repeat) pad.confirm = true;
				break;
			case SDLK_n:
				if (!repeat) pad.cancel = true;
				break;
			case SDLK_ESCAPE:
				if (!repeat) pad.menu = true;
				break;
			case SDLK_F5:
			case SDLK_i:
				if (!repeat) pad.map = true;
				break;
			case SDLK_g:
			case SDLK_w:
				if (!repeat) pad.save = true;
				break;
			case SDLK_c:
			case SDLK_l:
				if (!repeat) pad.load = true;
				break;
			case SDLK_v:
			case SDLK_F2:
				if (!repeat) pad.toggleGfx = true;
				break;
			case SDLK_F3:
				if (!repeat) pad.toggleFullscreen = true;
				break;
			case SDLK_m:
				if (!repeat) pad.toggleMute = true;
				break;
			case SDLK_RETURN:
			case SDLK_KP_ENTER:
				if (!repeat) pad.advanceTime = true;
				break;
			case SDLK_TAB:
				if (!repeat) pad.cycleCamera = true;
				break;
			case SDLK_0:
			case SDLK_1: case SDLK_2: case SDLK_3:
			case SDLK_4: case SDLK_5: case SDLK_6:
			case SDLK_7: case SDLK_8: case SDLK_9:
				if (!repeat) {
					int n = event.key.keysym.sym - SDLK_0;
					pad.lastNumberPressed = n;
					if (n >= 1 && n <= 7)
						pad.cameraTarget = n - 1;
				}
				break;
			default: break;
			}
			break;
		}

		case SDL_KEYUP:
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:    pad.up      = false; break;
			case SDLK_DOWN:  pad.down    = false; break;
			case SDLK_LEFT:  pad.left    = false; break;
			case SDLK_RIGHT: pad.right   = false; break;
			case SDLK_SPACE: pad.action  = false; break;
			case SDLK_q:     pad.actionQ = false; break;
			case SDLK_r:     pad.actionR = false; break;
			default: break;
			}
			break;

		case SDL_CONTROLLERDEVICEADDED:
			if (!gamepad) {
				gamepad = SDL_GameControllerOpen(event.cdevice.which);
				print("Gamepad conectado!\n");
			}
			break;

		case SDL_CONTROLLERDEVICEREMOVED:
			if (gamepad && event.cdevice.which ==
					SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad))) {
				SDL_GameControllerClose(gamepad);
				gamepad = NULL;
				print("Gamepad desconectado.\n");
			}
			break;

		case SDL_CONTROLLERBUTTONDOWN:
			switch (event.cbutton.button)
			{
			case SDL_CONTROLLER_BUTTON_DPAD_UP:    pad.up    = true; break;
			case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  pad.down  = true; break;
			case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  pad.left  = true; break;
			case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: pad.right = true; break;
			case SDL_CONTROLLER_BUTTON_A:          pad.action    = true; break;
			case SDL_CONTROLLER_BUTTON_B:          pad.confirm   = true; break;
			case SDL_CONTROLLER_BUTTON_X:          pad.cancel    = true; break;
			case SDL_CONTROLLER_BUTTON_Y:          pad.toggleGfx = true; break;
			case SDL_CONTROLLER_BUTTON_START:      pad.menu      = true; break;
			case SDL_CONTROLLER_BUTTON_BACK:       pad.map       = true; break;
			case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  pad.load = true; break;
			case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: pad.save = true; break;
			case SDL_CONTROLLER_BUTTON_LEFTSTICK:  pad.cycleCamera = true; break;
			default: break;
			}
			break;

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
			break;

		case SDL_CONTROLLERAXISMOTION:
			switch (event.caxis.axis)
			{
			case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
				pad.actionQ = (event.caxis.value > 16383);
				break;
			case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
				pad.actionR = (event.caxis.value > 16383);
				break;
			case SDL_CONTROLLER_AXIS_LEFTX:
				if      (event.caxis.value < -8192) { pad.left = true;  pad.right = false; }
				else if (event.caxis.value >  8192) { pad.right = true; pad.left  = false; }
				else                                { pad.left  = false; pad.right = false; }
				break;
			case SDL_CONTROLLER_AXIS_LEFTY:
				if      (event.caxis.value < -8192) { pad.up   = true;  pad.down  = false; }
				else if (event.caxis.value >  8192) { pad.down = true;  pad.up    = false; }
				else                                { pad.up   = false;  pad.down  = false; }
				break;
			default: break;
			}
			break;

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
		}
	}
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

		static auto lastLogic   = SDL_GetTicks64();
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
