#include <iostream>
#include <vector>
#include "system.h"

static System tmpSys;
System *const sys = &tmpSys;

void System::init()
{
	pad.up = false;
	pad.down = false;
	pad.left = false;
	pad.right = false;
	pad.button1 = false;
	pad.button2 = false;
	pad.button3 = false;
	pad.button4 = false;
	pad.start = false;

	gamepad = NULL;
	hapticDevice = NULL;
	Uint32 windowFlags = SDL_WINDOW_SHOWN;
	Uint32 initFlags = SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;  

	SDL_Init(SDL_INIT_VIDEO | initFlags | SDL_INIT_AUDIO);    
	
#ifdef ANDROID
	windowFlags = windowFlags | SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE;
#endif

	// Joypad
	if( SDL_NumJoysticks() < 1 ){		
		print("Warning: controller not found.\n");		
	}
	else{
		gamepad = SDL_GameControllerOpen(0);
		if (gamepad == NULL){
			print("Warning: Can't open SDL_GameControllerOpen(0)\n");
		}
		SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
	}
#ifndef __EMSCRIPTEN__
	if (SDL_InitSubSystem(SDL_INIT_HAPTIC) < 0){        
		print ("Error: Can't init SDL_InitSubSystem(SDL_INIT_HAPTIC).\n");
	}
  	else if (SDL_NumHaptics() > 0){
		haveHapticDevice = true;
		hapticDevice = SDL_HapticOpen(0);
		if (hapticDevice == NULL){
			print("Warning: SDL_HapticOpen(0) failed (non-fatal)\n");
		}
		else if (SDL_HapticRumbleInit(hapticDevice) != 0){			
			print("Warning: SDL_HapticRumbleInit failed (non-fatal)\n");
			SDL_HapticClose(hapticDevice);
			hapticDevice = NULL;
		}
	}
#endif

#ifdef RG350
	// soft haptic response
	if (SDL_HapticRumblePlay(hapticDevice, 0.30f /* Strength */, 10 /* Time */) < 0){		
        print("Warning: SDL_HapticRumbleStop failed\n");
	}
#endif
	
#ifdef ANDROID
	window = SDL_CreateWindow(WINDOW_TITLE, 0, 0, 0, 0, windowFlags);
	SDL_GetWindowSize(window, &w, &h);
#else
	window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, windowFlags);
	{
		int window_w, window_h=0;
		SDL_GetWindowSize(window, &window_w, &window_h);
		float scale_x = (float)window_w / TEXTURE_WIDTH;
		float scale_y = (float)window_h / TEXTURE_HEIGHT;

		float scale = fminf(scale_x, scale_y); // Escala uniforme más grande que cabe

		dstrect.w = (int)(TEXTURE_WIDTH * scale);
		dstrect.h = (int)(TEXTURE_HEIGHT * scale);

		dstrect.x = (window_w - dstrect.w) / 2;
		dstrect.y = (window_h - dstrect.h) / 2;
//	fprintf(stderr,"size  w %d h %d scale %f dw %d dh %d dx %d dy %d\n",
//					window_w, window_h, scale, 
//					dstrect.w , dstrect.h, dstrect.x , dstrect.y ); fflush(stderr);
}

#endif
	
	if (window == NULL){		
        print("ERROR: Could not create window.\n");
	}

	renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
	if( renderer == NULL ){
        print("ERROR: Could not create renderer.\n");		
	}

	if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ){
		print("ERROR: Mix_OpenAudio\n");
		fprintf(stderr,"ERROR: Mix_OpenAudio %s.\n",SDL_GetError());
	}
	int nc=Mix_AllocateChannels(static_cast<UINT8>(Abadia::SONIDOS::Count));
      	if (nc!=static_cast<UINT8>(Abadia::SONIDOS::Count)) {
		fprintf(stderr,"solo %d canales\n",nc); fflush(stderr);
	}

	//for (size_t i=0;i<std::size(Abadia::SOUND_FILE_NAMES);i++) esto requiere std=c++17
	for (auto i=0;i<static_cast<UINT8>(Abadia::SONIDOS::Count);i++)
	{
		fprintf(stderr,"loadwav %s\n", Abadia::SOUND_FILE_NAMES[i]); fflush(stderr);
		sounds.push_back(Mix_LoadWAV(Abadia::SOUND_FILE_NAMES[i]));
		if (sounds[i] == NULL){
			print("Error: can't read sound file\n");			
		}
	}

	surface = SDL_CreateRGBSurface(0, TEXTURE_WIDTH,TEXTURE_HEIGHT,32, rmask, gmask,bmask, amask);

	if (surface == NULL){		
	        print ("Error: Can't create surface.\n");
	}

	_pixels=static_cast<Uint32*>(surface->pixels);
	_pitch_pixels = surface->pitch / sizeof(UINT32);

//fprintf(stderr,"SDL_GetPixelFormatName %s\n",(char *)SDL_GetPixelFormatName(surface->format->format)); fflush(stderr);
	texture = SDL_CreateTextureFromSurface(renderer, surface);	
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

#ifdef __EMSCRIPTEN__
	interruptCounter=0;
	logicInterrupt=false;
	targetFrameTime=SDL_GetTicks64();
#endif
}

void System::hapticFeedback()
{
	if (!haveHapticDevice) return;
    
	if (SDL_HapticRumblePlay(hapticDevice, 0.80f /* Strength */, 200 /* Time */) < 0){
		print("Error: SDL_HapticRumblePlay failed\n");
	}
}

void System::quit()
{
	SDL_GameControllerClose(gamepad);
	SDL_HapticClose(hapticDevice);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);	

	for (size_t i=0;i<static_cast<int>(Abadia::SONIDOS::Count);i++) {
		Mix_FreeChunk(sounds[i]);
	}
	sounds.clear(); 

	SDL_Quit();	
}

void System::updateScreen()
{
#ifdef __EMSCRIPTEN__
	if (interruptCounter % 6 == 0 )
	{
#endif
	// Test this line with all the supported platforms. Fixes video problems with 
	// the raspberry pi with KMSDRM
	SDL_RenderClear(renderer);
#ifdef ANDROID
	SDL_RenderCopy(renderer, texture, NULL, NULL);
#else
	// ponemos fondo negro para que si al escalar
	// quedan bandas sin rellenar no queden en blanco
	// que no casa con el fondo real del juego
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderCopy(renderer, texture, NULL, &dstrect);
#endif
	SDL_RenderPresent(renderer);
#ifdef __EMSCRIPTEN__
	}
#endif
}

void System::stopSound(Abadia::SONIDOS index)
{
	auto i=static_cast<int>(index);
	assert(i < static_cast<int>(Abadia::SONIDOS::Count));
	Mix_HaltChannel(i);
}
void System::playSound(Abadia::SONIDOS index, bool loop)
{
	auto i=static_cast<int>(index);
	assert(i < static_cast<int>(Abadia::SONIDOS::Count));
	Mix_PlayChannel(i, sounds[i], loop);
}
void System::handleEvents()
{
	SDL_Event event;	
	while(SDL_PollEvent(&event))
	{
		SDL_ControllerButtonEvent ev = event.cbutton;
		if( event.type == SDL_QUIT ){
			exit = true;
		}
		else if(event.type == SDL_KEYDOWN)
		{
			switch(event.key.keysym.sym)
			{
#if RG350                
				case SDLK_ESCAPE:
#else
                case SDLK_1:
#endif                    
					informationMode = !informationMode;
					break;
				case SDLK_LEFT:
					pad.left = true;
					break;
				case SDLK_RIGHT:
					pad.right = true;
					break;
				case SDLK_UP:
					pad.up = true;
					break;
				case SDLK_DOWN:
					pad.down = true;
					break;				
#if RG350                    
				case SDLK_LSHIFT: //Y (S)
#else
                case SDLK_RETURN:
#endif		
					pad.button3 = true;
					break;
#if RG350                    
				case SDLK_LALT: //B (N)
#else
                case SDLK_n:
#endif                    
					pad.button4 = true;
					break;	
				case SDLK_SPACE: //X (Drop)
					pad.button1 = true;
					break;
				case SDLK_LCTRL: //A (?)
					pad.button2 = true;
					break;
#if RG350
				case SDLK_RETURN:
#else
                case SDLK_2:
#endif 
					pad.start = true;
					break;									
				default:
					break;
			}
		}
		else if(event.type == SDL_KEYUP)
		{
			switch(event.key.keysym.sym)
			{
				case SDLK_LEFT:
					pad.left = false;
					break;
				case SDLK_RIGHT:
					pad.right = false;
					break;
				case SDLK_UP:
					pad.up = false;
					break;
				case SDLK_DOWN:
					pad.down = false;
					break;
#if RG350                    
				case SDLK_LSHIFT: //Y (S)
#else
                case SDLK_RETURN:
#endif                    
					pad.button3 = false;
					break;
#if RG350                    
				case SDLK_LALT: //B (N)
#else
                case SDLK_n:
#endif 
					pad.button4 = false;
					break;	
				case SDLK_SPACE: //X (Drop)
					pad.button1 = false;
					break;
				case SDLK_LCTRL: //A (?)
					pad.button2 = false;
					break;
#if RG350
				case SDLK_RETURN:
#else
                case SDLK_2:
#endif                    
					pad.start = true;
					break;		
				default:
					break;
			}
		}
		else if (event.type == SDL_CONTROLLERBUTTONDOWN)
		{
			
			switch(ev.button)
			{
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					pad.left = true;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					pad.right = true;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					pad.up = true;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					pad.down = true;
					break;
				case SDL_CONTROLLER_BUTTON_A: //Y (S)
					pad.button3 = true;
					break;
				case SDL_CONTROLLER_BUTTON_X: //B (N)
					pad.button4 = true;
					break;	
				case SDL_CONTROLLER_BUTTON_Y: //X (Drop)
					pad.button1 = true;
					break;
				case SDL_CONTROLLER_BUTTON_B: //A (?)
					pad.button2 = true;
					break;
				case SDL_CONTROLLER_BUTTON_START:
					pad.start = true;
					break;
				case SDL_CONTROLLER_BUTTON_BACK :
					informationMode = !informationMode;					
					break;		
				default:
					break;
			}
		}
		else if (event.type == SDL_CONTROLLERBUTTONUP)
		{			
			switch(ev.button)
			{
				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					pad.left = false;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					pad.right = false;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					pad.up = false;
					break;
				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					pad.down = false;
					break;
				case SDL_CONTROLLER_BUTTON_A: //Y (S)
					pad.button3 = false;
					break;
				case SDL_CONTROLLER_BUTTON_X: //B (N)
					pad.button4 = false;
					break;	
				case SDL_CONTROLLER_BUTTON_Y: //X (Drop)
					pad.button1 = false;
					break;
				case SDL_CONTROLLER_BUTTON_B: //A (?)
					pad.button2 = false;
					break;
				case SDL_CONTROLLER_BUTTON_START:
					pad.start = false;
					break;
				case SDL_CONTROLLER_BUTTON_BACK :					
				default:
					break;
			}
		}
		//else if (event.type == SDL_WINDOWEVENT_SIZE_CHANGED)
		else if (event.type ==  SDL_WINDOWEVENT && 
				(event.window.event == SDL_WINDOWEVENT_RESIZED ||
				event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			)
		{
			int window_w, window_h=0;
			SDL_GetWindowSize(window, &window_w, &window_h);
			float scale_x = (float)window_w / TEXTURE_WIDTH;
			float scale_y = (float)window_h / TEXTURE_HEIGHT;

			float scale = fminf(scale_x, scale_y); // Escala uniforme más grande que cabe

			dstrect.w = (int)(TEXTURE_WIDTH * scale);
			dstrect.h = (int)(TEXTURE_HEIGHT * scale);

			dstrect.x = (window_w - dstrect.w) / 2;
			dstrect.y = (window_h - dstrect.h) / 2;

//			fprintf(stderr,"size changed w %d h %d scale %f dw %d dh %d dx %d dy %d\n",
//					window_w, window_h, scale, 
//					dstrect.w , dstrect.h, dstrect.x , dstrect.y ); fflush(stderr);
		}
	}
}

void System::setFastSpeed()
{
	minimumFrameTime = SCROLL_FRAME_TIME;
}	
void System::setNormalSpeed()
{
	minimumFrameTime = GAME_FRAME_TIME;
}
Uint32 System::RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	return SDL_MapRGBA(surface->format, r,g,b,a);
}
void System::updateTexture()
{
	SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
}
void System::exitGame()
{
	exit = true;
}
void System::print(const std::string message)
{
	#ifdef ANDROID
		__android_log_print(ANDROID_LOG_DEBUG, "ABBEY", "%s\n", message.c_str());
	#else
		std::cout << message;
	#endif
}

void System::initFrame()
{
	frameTime = SDL_GetTicks64();
#ifdef __EMSCRIPTEN__
	interruptCounter++;
	if (frameTime>=targetFrameTime) logicInterrupt=true; 
#endif
}

void System::endFrame()
{
#ifndef __EMSCRIPTEN__
	//cap the frame rate
	if (SDL_GetTicks64() - frameTime < sys->minimumFrameTime) {
		SDL_Delay(sys->minimumFrameTime - (SDL_GetTicks64() - frameTime));
	}
#else
	if (logicInterrupt) {
		targetFrameTime+=0x24*(1000./300.); // 300 ints x second, y 36 iteraciones por logica
		if (targetFrameTime<=frameTime) targetFrameTime=SDL_GetTicks64()+5;
		logicInterrupt=false; 
		/*
		// log IPS 
		static auto lastLogic = SDL_GetTicks64();
		static int framesLogic = 0;
		framesLogic++;
		if (SDL_GetTicks64() - lastLogic > 1000) {
			if (framesLogic!=9) SDL_Log("IPS: %d Debería ser 9", framesLogic);
			framesLogic = 0;
			lastLogic = SDL_GetTicks64();
		}  */
	}
#endif	
/*
	// Log FPS
	static auto last = SDL_GetTicks64();
	static int frames = 0;
	frames++;
	if (SDL_GetTicks64() - last > 1000) {
		if (frames!=60) SDL_Log("FPS: %d", frames);
		frames = 0;
		last = SDL_GetTicks64();
	}
	*/
}
