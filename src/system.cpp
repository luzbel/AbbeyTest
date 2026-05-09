#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include "system.h"

static System tmpSys;
System *const sys = &tmpSys;

// ----------------------------------------------------------------------------
// OpenGL
// GL 1.x (glGenTextures, glBindTexture, glTexImage2D, glEnable, glViewport,
// glClear, glClearColor, glTexParameteri, glTexSubImage2D, glBegin/glEnd,
// glTexCoord2f, glVertex2f) están en el header base — se llaman directamente.
// Solo los shaders (GL 2.0) necesitan punteros de función en desktop.
// ----------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__
#include <GLES2/gl2.h>
// En Emscripten las funciones GL2 están disponibles directamente
#define _gl_CreateShader      glCreateShader
#define _gl_ShaderSource      glShaderSource
#define _gl_CompileShader     glCompileShader
#define _gl_CreateProgram     glCreateProgram
#define _gl_AttachShader      glAttachShader
#define _gl_LinkProgram       glLinkProgram
#define _gl_UseProgram        glUseProgram
#define _gl_GetUniformLocation glGetUniformLocation
#define _gl_Uniform1i         glUniform1i
#define _gl_Uniform1f         glUniform1f
#define _gl_Uniform2f         glUniform2f
#define _gl_GetAttribLocation glGetAttribLocation
#define _gl_GenBuffers               glGenBuffers
#define _gl_BindBuffer               glBindBuffer
#define _gl_BufferData               glBufferData
#define _gl_EnableVertexAttribArray  glEnableVertexAttribArray
#define _gl_VertexAttribPointer      glVertexAttribPointer
#define _gl_DisableVertexAttribArray glDisableVertexAttribArray
#define _gl_DeleteBuffers            glDeleteBuffers
#define _gl_GetProgramiv glGetProgramiv
#else
#include <SDL2/SDL_opengl.h>
// Punteros solo para las funciones GL 2.0 (shaders)
static PFNGLCREATESHADERPROC       _gl_CreateShader;
static PFNGLSHADERSOURCEPROC       _gl_ShaderSource;
static PFNGLCOMPILESHADERPROC      _gl_CompileShader;
static PFNGLCREATEPROGRAMPROC      _gl_CreateProgram;
static PFNGLATTACHSHADERPROC       _gl_AttachShader;
static PFNGLLINKPROGRAMPROC        _gl_LinkProgram;
static PFNGLUSEPROGRAMPROC         _gl_UseProgram;
static PFNGLGETUNIFORMLOCATIONPROC _gl_GetUniformLocation;
static PFNGLUNIFORM1IPROC          _gl_Uniform1i;
static PFNGLUNIFORM1FPROC          _gl_Uniform1f;
static PFNGLUNIFORM2FPROC          _gl_Uniform2f;
static PFNGLGETATTRIBLOCATIONPROC _gl_GetAttribLocation;
static PFNGLGENBUFFERSPROC             _gl_GenBuffers;
static PFNGLBINDBUFFERPROC             _gl_BindBuffer;
static PFNGLBUFFERDATAPROC             _gl_BufferData;
static PFNGLENABLEVERTEXATTRIBARRAYPROC _gl_EnableVertexAttribArray;
static PFNGLVERTEXATTRIBPOINTERPROC    _gl_VertexAttribPointer;
static PFNGLDISABLEVERTEXATTRIBARRAYPROC _gl_DisableVertexAttribArray;
static PFNGLDELETEBUFFERSPROC          _gl_DeleteBuffers;
static PFNGLGETPROGRAMIVPROC _gl_GetProgramiv;
#endif

// Estado GL — el contexto lo gestiona SDL_Renderer internamente
static GLuint shaderProgram  = 0;
static GLint  efectoLocation = -1;
static GLint filtroLocation = -1;
static GLint texSizeLocation = -1;
static GLint textureLocation = -1;
static GLint textureMapLocation = -1;
static GLint textureMenuLocation = -1;
static GLint textureIntroLocation = -1;
static GLint flipTLocation = -1;

// ----------------------------------------------------------------------------
// initGLPointers — solo en desktop, solo funciones GL 2.0
// ----------------------------------------------------------------------------

#ifndef __EMSCRIPTEN__
static bool initGLPointers()
{
    _gl_CreateShader       = (PFNGLCREATESHADERPROC)       SDL_GL_GetProcAddress("glCreateShader");
    _gl_ShaderSource       = (PFNGLSHADERSOURCEPROC)       SDL_GL_GetProcAddress("glShaderSource");
    _gl_CompileShader      = (PFNGLCOMPILESHADERPROC)      SDL_GL_GetProcAddress("glCompileShader");
    _gl_CreateProgram      = (PFNGLCREATEPROGRAMPROC)      SDL_GL_GetProcAddress("glCreateProgram");
    _gl_AttachShader       = (PFNGLATTACHSHADERPROC)       SDL_GL_GetProcAddress("glAttachShader");
    _gl_LinkProgram        = (PFNGLLINKPROGRAMPROC)        SDL_GL_GetProcAddress("glLinkProgram");
    _gl_UseProgram         = (PFNGLUSEPROGRAMPROC)         SDL_GL_GetProcAddress("glUseProgram");
    _gl_GetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SDL_GL_GetProcAddress("glGetUniformLocation");
    _gl_Uniform1i          = (PFNGLUNIFORM1IPROC)          SDL_GL_GetProcAddress("glUniform1i");
    _gl_Uniform1f          = (PFNGLUNIFORM1FPROC)          SDL_GL_GetProcAddress("glUniform1f");
    _gl_Uniform2f          = (PFNGLUNIFORM2FPROC)          SDL_GL_GetProcAddress("glUniform2f");
    _gl_GetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SDL_GL_GetProcAddress("glGetAttribLocation");
    _gl_GenBuffers              = (PFNGLGENBUFFERSPROC)              SDL_GL_GetProcAddress("glGenBuffers");
_gl_BindBuffer              = (PFNGLBINDBUFFERPROC)              SDL_GL_GetProcAddress("glBindBuffer");
_gl_BufferData              = (PFNGLBUFFERDATAPROC)              SDL_GL_GetProcAddress("glBufferData");
_gl_EnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glEnableVertexAttribArray");
_gl_VertexAttribPointer     = (PFNGLVERTEXATTRIBPOINTERPROC)     SDL_GL_GetProcAddress("glVertexAttribPointer");
_gl_DisableVertexAttribArray= (PFNGLDISABLEVERTEXATTRIBARRAYPROC)SDL_GL_GetProcAddress("glDisableVertexAttribArray");
_gl_DeleteBuffers           = (PFNGLDELETEBUFFERSPROC)           SDL_GL_GetProcAddress("glDeleteBuffers");
_gl_GetProgramiv = (PFNGLGETPROGRAMIVPROC)           SDL_GL_GetProcAddress("glGetProgramiv");

    if (!_gl_CreateShader || !_gl_CreateProgram || !_gl_UseProgram) {
        return false;
    }
    return true;
}
#endif

void System::initShader(int efectoPaleta)
{
	SDL_Log("initShader 1\n");
    if (!useWebGL) return;
	SDL_Log("initShader 2\n");

    const char* vertexSource = R"(
        attribute vec2 aPosition;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        void main() {
            vTexCoord   = aTexCoord;
            gl_Position = vec4(aPosition, 0.0, 1.0);
        }
    )";
/*
    const char* fragmentSource = 
#include "shader_common.glsl"
#include "shader_xbr.glsl"
#include "shader_main.glsl"
*/
	std::string fragmentSource = 
		std::string("#define PAGE_LEFT ") + "uTextureMenu" + "\n" + 
		std::string("#define PAGE_RIGHT ") + "uTextureIntro" + "\n" +
		std::string("#define NEXT_PAGE_LEFT ") + "uTextureMap" + "\n" + 
		std::string("#define NEXT_PAGE_RIGHT ") + "uTexture" + "\n" +
#include "shader_common.glsl"
#include "shader_xbr.glsl"
//#include "shader_book.glsl"
#include "shader_book_cover.glsl"
#include "shader_main.glsl"
//#include "shaderQWEN.glsl"

    std::cout << "DEBUG SHADER CONTENT:\n" << fragmentSource << "\n---END---" << std::endl;



    auto compileShader = [&](GLenum type, const char* src) -> GLuint {
        GLuint s = _gl_CreateShader(type);
        _gl_ShaderSource(s, 1, &src, nullptr);
        _gl_CompileShader(s);
        return s;
    };

    GLuint vert = compileShader(GL_VERTEX_SHADER,   vertexSource);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());

    shaderProgram = _gl_CreateProgram();
    _gl_AttachShader(shaderProgram, vert);
    _gl_AttachShader(shaderProgram, frag);
    _gl_LinkProgram(shaderProgram);

        GLint status;
    _gl_GetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    SDL_Log("status1 %d\n",status);
    if (status == GL_FALSE) {
    SDL_Log("**********\n************\nCAGADA\n**********\n********\nstatus1 %d\n",status);
     //   char log[512]; _gl_GetProgramInfoLog(shaderProgram, sizeof(log), nullptr, log);
//        SDL_Log("ERROR ENLACE SHADER: %s", log);
        useWebGL = false; return;
    }
    SDL_Log("status2 %d\n",status);

    efectoLocation = _gl_GetUniformLocation(shaderProgram, "uEfecto");
    SDL_Log("initShader OK — shaderProgram=%u efectoLocation=%d", shaderProgram, efectoLocation);
    texSizeLocation = _gl_GetUniformLocation(shaderProgram, "uTexSize");
    filtroLocation = _gl_GetUniformLocation(shaderProgram, "uFiltro");
    textureLocation = _gl_GetUniformLocation(shaderProgram, "uTexture");
    textureMapLocation = _gl_GetUniformLocation(shaderProgram, "uTextureMap");
    textureMenuLocation = _gl_GetUniformLocation(shaderProgram, "uTextureMenu");
    textureIntroLocation = _gl_GetUniformLocation(shaderProgram, "uTextureIntro");
    flipTLocation = _gl_GetUniformLocation(shaderProgram, "uFlipT");
}

// ----------------------------------------------------------------------------
// init
// ----------------------------------------------------------------------------

void System::init()
{
    gamepad      = nullptr;
    hapticDevice = nullptr;

    // SDL_WINDOW_OPENGL siempre: si useWebGL acaba siendo false tras loadConfig
    // simplemente no usaremos el contexto GL, pero tenerlo no hace daño.
    Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
    Uint32 initFlags   = SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;

    SDL_Init(SDL_INIT_VIDEO | initFlags | SDL_INIT_AUDIO);

#ifdef ANDROID
    windowFlags |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_RESIZABLE;
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

    // -------------------------------------------------------------------
    // Ventana
    // -------------------------------------------------------------------
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
    if (!window) print("ERROR: Could not create window.\n");

    // -------------------------------------------------------------------
    // Surface — antes de loadConfig, los callbacks de paleta la necesitan
    // -------------------------------------------------------------------
    surface = SDL_CreateRGBSurface(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 32,
                                   rmask, gmask, bmask, amask);
    surfaceMap = SDL_CreateRGBSurface(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 32,
                                   rmask, gmask, bmask, amask);
    surfaceMenu = SDL_CreateRGBSurface(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 32,
                                   rmask, gmask, bmask, amask);
    surfaceIntro = SDL_CreateRGBSurface(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 32,
                                   rmask, gmask, bmask, amask);
    if (!surface || !surfaceMap || !surfaceMenu || !surfaceIntro) print("Error: Can't create surfaces.\n");
    _pixels       = static_cast<Uint32*>(surface->pixels);
    _pixelsMenu   = static_cast<Uint32*>(surfaceMenu->pixels);
    _pixelsMap    = static_cast<Uint32*>(surfaceMap->pixels);
    _pixelsIntro  = static_cast<Uint32*>(surfaceIntro->pixels);
    _pitch_pixels = surface->pitch / sizeof(UINT32);

    // tiene que inicializarse antes del setcallback de setmute
    // porque en escritorio se controla pero en emscripten falla
    // -------------------------------------------------------------------
    // Audio
    // -------------------------------------------------------------------
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

    // -------------------------------------------------------------------
    // Callbacks antes de loadConfig para que se disparen al cargar
    // -------------------------------------------------------------------
    mute.setCallback([this](bool v) { setMute(v); });
SDL_Log("antes de  loadConfig\n");
    // -------------------------------------------------------------------
    // loadConfig — aquí se lee useWebGL real del fichero
    // -------------------------------------------------------------------
    loadConfig();
SDL_Log("despues de  loadConfig\n");

    // -------------------------------------------------------------------
    // Renderer y recursos de pantalla — DESPUÉS de loadConfig
    // -------------------------------------------------------------------
    // Renderer siempre OpenGL+TargetTexture — el hint fuerza el backend GL
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    renderer = SDL_CreateRenderer(window, -1,
                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (!renderer) print("ERROR: Could not create renderer.\n");
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Texture target: el juego renderiza aquí, luego la presentamos
    // SDL_TEXTUREACCESS_TARGET permite SDL_SetRenderTarget
    // SDL_TEXTUREACCESS_STREAMING permitiría UpdateTexture directo
    // Usamos STREAMING para poder hacer UpdateTexture desde surface->pixels
    texture = SDL_CreateTexture(renderer, surface->format->format,
                  SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    textureMap = SDL_CreateTexture(renderer, surfaceMap->format->format,
                  SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    textureMenu = SDL_CreateTexture(renderer, surfaceMenu->format->format,
                  SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    textureIntro = SDL_CreateTexture(renderer, surfaceIntro->format->format,
                  SDL_TEXTUREACCESS_STREAMING, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    if (!texture || !textureMap || !textureMenu || !textureIntro) print("ERROR: Could not create textures.\n");

//    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Punteros GL e initShader — después del renderer, que ya creó el contexto GL
    if (useWebGL) {
#ifndef __EMSCRIPTEN__
        SDL_RendererInfo info;
        SDL_GetRendererInfo(renderer, &info);
        if (strncmp(info.name, "opengl", 6) == 0) {
            if (!initGLPointers()) {
                print("WARNING: No se pudieron cargar punteros GL. Forzando SW.\n");
                useWebGL = false;
            }
        } else {
            print("WARNING: Renderer no es OpenGL. Forzando SW.\n");
            useWebGL = false;
        }
#endif
        if (useWebGL) initShader(paletaEfecto);
    }


#ifdef __EMSCRIPTEN__
    interruptCounter = 0;
    logicInterrupt   = false;
    targetFrameTime  = SDL_GetTicks64();
#endif
}

/*
 
TODO: probar en vez de sumar 0.1

//El shader solo renderiza t ∈ [0,1]. Para que parezca un libro real, no lineales uFlipT. Usa una curva de aceleración en el bucle de actualización:

// Ejemplo C++/Emscripten
float flipDuration = 0.45f; // segundos
float elapsed = 0.0f;
bool animating = true;

void UpdateBookCover(float dt) {
    if (!animating) return;
    elapsed += dt;
    float rawT = std::min(elapsed / flipDuration, 1.0f);
    // Ease-in-out cuadrática: arranca suave, frena al llegar al plano
    uFlipT = rawT * rawT * (3.0f - 2.0f * rawT);

    if (rawT >= 1.0f) animating = false; // 🛑 Detiene la animación
} 
*/

void System::updateScreen()
{
#ifdef __EMSCRIPTEN__
    if (interruptCounter % 6 == 0) {
#endif

//    SDL_UpdateTexture(texture,    nullptr, surface->pixels, surface->pitch);
//    SDL_UpdateTexture(textureMap, nullptr, surfaceMap->pixels, surface->pitch);
//    SDL_UpdateTexture(textureMenu, nullptr, surfaceMenu->pixels, surface->pitch);
//TODO si no está en modo webGL igual se puede simplificar
	if((bool)sys->useWebGL) {
		switch(_state)
		{
			case Abadia::STATES::INTRO:
				SDL_UpdateTexture(textureIntro, nullptr, surfaceIntro->pixels, surface->pitch);
				break;
			case Abadia::STATES::CONFIG_GFX:
			case Abadia::STATES::CONFIG_SND:
			case Abadia::STATES::HELP:
			case Abadia::STATES::HELP_INTRODUCCION:
			case Abadia::STATES::HELP_MANEJO:
			case Abadia::STATES::HELP_AYUDAS:
			case Abadia::STATES::HELP_CAMARAS:
			case Abadia::STATES::HELP_REFERENCIAS:
			case Abadia::STATES::CONFIG:
			case Abadia::STATES::ASK_EXIT:
			case Abadia::STATES::ASK_CONTINUE:
			case Abadia::STATES::ASK_NEW_GAME:
			case Abadia::STATES::LANGUAGE:
			case Abadia::STATES::MENU:
			case Abadia::STATES::LOAD:
			case Abadia::STATES::SAVE:
				SDL_UpdateTexture(textureMenu, nullptr, surfaceMenu->pixels, surfaceMenu->pitch);
				break;
			case Abadia::STATES::SCROLL:
			case Abadia::STATES::HELP_MANEJO_PERGAMINO:
			case Abadia::STATES::ENDING:
				SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
				break;
			case Abadia::STATES::PLAY:
				SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
				SDL_UpdateTexture(textureMap, nullptr, surfaceMap->pixels, surface->pitch);
				break;
		}
    } else
	SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);

    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderClear(renderer);
#define EBUGSHADER

#ifdef DEBUGSHADER
    { // solo para depurar
    Uint32* p = (Uint32*)surface->pixels;
    int w = TEXTURE_WIDTH, h = TEXTURE_HEIGHT;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            // Mitad superior: AMARILLO puro
            // Mitad inferior: AZUL puro
            // Línea central vertical: BLANCA (para detectar desplazamiento) 
	    /*
            if (x == w/2) p[y * _pitch_pixels + x] = 0xFFFFFFFF;
            else if (y < h/2) p[y * _pitch_pixels + x] = 0x00FF0000; // Azul
            else p[y * _pitch_pixels + x] = 0x0000FFFF;              // Amarillo 
 */
p[y * _pitch_pixels + x] = 0x000000FF; 	
        }
    }
    // Actualiza textura antes de dibujar
    SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
    SDL_UpdateTexture(textureMap, nullptr, surface->pixels, surface->pitch);
    }
#endif

/*    if (useWebGL && shaderProgram) {
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT); // ✅ Limpia framebuffer raw correctamente

    float tw, th;
    glActiveTexture(GL_TEXTURE0); // ✅ Asegura unidad 0
    SDL_GL_BindTexture(texture, &tw, &th);
    _gl_Uniform1i(textureLocation, 0); // ✅ Vincula textura al sampler
				       
    float twMenu, thMenu;
    glActiveTexture(GL_TEXTURE1); // ✅ Asegura unidad 0
    SDL_GL_BindTexture(textureMenu, &twMenu, &thMenu);
    _gl_Uniform1i(textureMenuLocation, 0); // ✅ Vincula textura al sampler
 
//    SDL_GL_BindTexture(textureMap, &tw, &th);
    SDL_GL_BindTexture(textureMenu, &tw, &th);
//    SDL_Log("****SDL_GL_BindTexture: tw=%.4f th=%.4f****", tw, th);

    int ww, wh;
    SDL_GetWindowSize(window, &ww, &wh);
    float x0 = (2.f * dstrect.x                      / ww) - 1.f;
    float x1 = (2.f * (dstrect.x + dstrect.w)        / ww) - 1.f;
    float y0 = 1.f - (2.f * dstrect.y                / wh);
    float y1 = 1.f - (2.f * (dstrect.y + dstrect.h)  / wh);

    GLfloat verts[] = { x0,y0,  x1,y0,  x0,y1,  x1,y1 };
    GLfloat uvs[]   = { 0.f,0.f, 1.f,0.f, 0.f,1.f, 1.f,1.f }; // ✅ [0,1] estricto

    GLint oldProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    _gl_UseProgram(shaderProgram);

//    _gl_Uniform1i(textureMapLocation, 0); // ✅ Vincula textura al sampler
    _gl_Uniform1i(textureMenuLocation, 0); // ✅ Vincula textura al sampler
    //_gl_Uniform1i(efectoLocation, (int)paletaEfecto);
    _gl_Uniform1f(efectoLocation, (float)paletaEfecto); // en versiones antiguas de opengl esto tiene que ser float
    _gl_Uniform1i(filtroLocation, (int)filtro);
    _gl_Uniform2f(texSizeLocation, (float)TEXTURE_WIDTH, (float)TEXTURE_HEIGHT);

    GLint posLoc = _gl_GetAttribLocation(shaderProgram, "aPosition");
    GLint uvLoc  = _gl_GetAttribLocation(shaderProgram, "aTexCoord");
//    SDL_Log("posLoc=%d uvLoc=%d efectoLoc =%d filtroLoc=%d texSizeLoc=%d",
//            posLoc, uvLoc, efectoLocation, filtroLocation, texSizeLocation);
//SDL_Log("uEfecto enviado como float: %f", (float)uEfecto);

    GLuint vbo[2];
    _gl_GenBuffers(2, vbo);

    _gl_BindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    _gl_BufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);
    _gl_EnableVertexAttribArray(posLoc);
    _gl_VertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    _gl_BindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    _gl_BufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STREAM_DRAW);
    _gl_EnableVertexAttribArray(uvLoc);
    _gl_VertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    _gl_DisableVertexAttribArray(posLoc);
    _gl_DisableVertexAttribArray(uvLoc);
    _gl_DeleteBuffers(2, vbo);

    glActiveTexture(GL_TEXTURE0); // ✅ Asegura unidad 0
    SDL_GL_UnbindTexture(texture);
//    SDL_GL_UnbindTexture(textureMap);
    glActiveTexture(GL_TEXTURE1); // ✅ Asegura unidad 0
    SDL_GL_UnbindTexture(textureMenu);
    SDL_GL_SwapWindow(window);
    _gl_UseProgram(oldProgram);
}
*/
if (useWebGL && shaderProgram) {
    int ww, wh;
    SDL_GetWindowSize(window, &ww, &wh);
    glViewport(0, 0, ww, wh);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Unidad 0: uTexture (página derecha)
    float tw, th;
    glActiveTexture(GL_TEXTURE0);
    SDL_GL_BindTexture(texture, &tw, &th);

    // Unidad 1: uTextureMenu (página izquierda)
    float tw1, th1;
    glActiveTexture(GL_TEXTURE1);
    SDL_GL_BindTexture(textureMenu, &tw1, &th1);

	// Unidad 2: uTextureMap 
    float tw2, th2;
    glActiveTexture(GL_TEXTURE2);
    SDL_GL_BindTexture(textureMap, &tw2, &th2);

	// Unidad 3: uTextureIntro
    float tw3, th3;
    glActiveTexture(GL_TEXTURE3);
    SDL_GL_BindTexture(textureIntro, &tw3, &th3);

    glActiveTexture(GL_TEXTURE0);

    GLint oldProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);
    _gl_UseProgram(shaderProgram);

    _gl_Uniform1i(textureLocation,    0);
    _gl_Uniform1i(textureMenuLocation, 1);
    _gl_Uniform1i(textureMapLocation, 2);
    _gl_Uniform1i(textureIntroLocation, 3);
//    static float tmp=0.0f;
//    tmp=tmp+0.01f;
//SDL_Log("tmp %f\n", tmp);
//    _gl_Uniform1f(flipTLocation,     (float)tmp);
	uFlipT+=0.01f; // probar el UpdateBookCover que propone QWEN
    _gl_Uniform1f(flipTLocation,     (float)uFlipT);
    _gl_Uniform1f(efectoLocation,     (float)paletaEfecto);
    _gl_Uniform1i(filtroLocation,     (int)filtro);
    _gl_Uniform2f(texSizeLocation,    (float)TEXTURE_WIDTH, (float)TEXTURE_HEIGHT);

    GLfloat verts[] = { -1.f,1.f,  1.f,1.f,  -1.f,-1.f,  1.f,-1.f };
    GLfloat uvs[]   = {  0.f,0.f,  1.f,0.f,   0.f, 1.f,  1.f, 1.f };

    GLint posLoc = _gl_GetAttribLocation(shaderProgram, "aPosition");
    GLint uvLoc  = _gl_GetAttribLocation(shaderProgram, "aTexCoord");

    GLuint vbo[2];
    _gl_GenBuffers(2, vbo);
    _gl_BindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    _gl_BufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);
    _gl_EnableVertexAttribArray(posLoc);
    _gl_VertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    _gl_BindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    _gl_BufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STREAM_DRAW);
    _gl_EnableVertexAttribArray(uvLoc);
    _gl_VertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    _gl_DisableVertexAttribArray(posLoc);
    _gl_DisableVertexAttribArray(uvLoc);
    _gl_DeleteBuffers(2, vbo);

    glActiveTexture(GL_TEXTURE3);
    SDL_GL_UnbindTexture(textureIntro);
    glActiveTexture(GL_TEXTURE2);
    SDL_GL_UnbindTexture(textureMap);
    glActiveTexture(GL_TEXTURE1);
    SDL_GL_UnbindTexture(textureMenu);
    glActiveTexture(GL_TEXTURE0);
    SDL_GL_UnbindTexture(texture);

    SDL_GL_SwapWindow(window);
    _gl_UseProgram(oldProgram);
}
else {
//     SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
     // Modo SW: pipeline SDL_Renderer clásico
#ifdef ANDROID
     SDL_RenderCopy(renderer, texture, nullptr, nullptr);
#else
     SDL_RenderCopy(renderer, texture, nullptr, &dstrect); 
#endif
     SDL_RenderPresent(renderer);
}


#ifdef __EMSCRIPTEN__
    }
#endif
}

//void System::updateTexture()
//{
//666    SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
//666    SDL_UpdateTexture(textureMap, nullptr, surface->pixels, surface->pitch);
//}
// ----------------------------------------------------------------------------
// Utilidades
// ----------------------------------------------------------------------------

Uint32 System::RGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    return SDL_MapRGBA(surface->format, r, g, b, a);
}


// ----------------------------------------------------------------------------
// Frame timing
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

//#include "system.fragment"
// cortar por aquí para pasar a LLM
// solo las partes gráficas y consumir menos tokens


// quit
// ----------------------------------------------------------------------------

void System::quit()
{
    saveConfig();

    SDL_GameControllerClose(gamepad);
    SDL_HapticClose(hapticDevice);

    if (texture)  SDL_DestroyTexture(texture);
    if (textureMap)  SDL_DestroyTexture(textureMap);
    if (textureMenu)  SDL_DestroyTexture(textureMenu);
    if (textureIntro)  SDL_DestroyTexture(textureIntro);
    if (renderer) SDL_DestroyRenderer(renderer);
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
#else
    return "config.txt";
#endif
}

void System::loadConfig()
{
	SDL_Log("10\n");
    delete _configReader;
	SDL_Log("20\n");
    _configReader = new ConfigReader(configPath());
	SDL_Log("30\n");

    mute        .bind(_configReader);
    GraficosCPC .bind(_configReader);
    idioma      .bind(_configReader);
    filtro      .bind(_configReader);
    scanlines   .bind(_configReader);
    useWebGL    .bind(_configReader);
    paletaEfecto.bind(_configReader);
	SDL_Log("40\n");

    if (_configReader->parse()) {
	SDL_Log("50\n");
        mute        .load();
	SDL_Log("51\n");
        GraficosCPC .load();
	SDL_Log("52\n");
        idioma      .load();
        filtro      .load();
        scanlines   .load();
        useWebGL    .load();
        paletaEfecto.load();
    }
	SDL_Log("60\n");

    loadSlotDates();
	SDL_Log("70\n");
}

void System::saveConfig()
{
    if (!_configReader) return;

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
	SDL_Log("loadSlotDates\n");
    if (!_configReader) return;
	SDL_Log("loadSlotDates hay _configReader y NUM_SLOTS es %d\n",NUM_SLOTS);
    for (int i = 0; i < NUM_SLOTS; i++) {
        std::string token = "SAVEX";
        token[4] = static_cast<char>('0' + i);
	SDL_Log("loadSlotDates antes de getValue\n");
        slotDates[i] = _configReader->getValue(token);
	SDL_Log("loadSlotDates despues de getValue\n");
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
// Audio
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
// Haptic
// ----------------------------------------------------------------------------

void System::hapticFeedback()
{
    if (!haveHapticDevice) return;
    if (SDL_HapticRumblePlay(hapticDevice, 0.80f, 200) < 0)
        print("Error: SDL_HapticRumblePlay failed\n");
}

// ----------------------------------------------------------------------------
// Screen / rendering
// ----------------------------------------------------------------------------

void System::toggleFullscreenMode()
{
    fullscreen = !fullscreen;
    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
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
            case SDL_CONTROLLER_BUTTON_DPAD_UP:       pad.up        = true; break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     pad.down      = true; break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     pad.left      = true; break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    pad.right     = true; break;
            case SDL_CONTROLLER_BUTTON_A:             pad.action    = true; break;
            case SDL_CONTROLLER_BUTTON_B:             pad.confirm   = true; break;
            case SDL_CONTROLLER_BUTTON_X:             pad.cancel    = true; break;
            case SDL_CONTROLLER_BUTTON_Y:             pad.toggleGfx = true; break;
            case SDL_CONTROLLER_BUTTON_START:         pad.menu      = true; break;
            case SDL_CONTROLLER_BUTTON_BACK:          pad.map       = true; break;
            case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  pad.load      = true; break;
            case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: pad.save      = true; break;
            case SDL_CONTROLLER_BUTTON_LEFTSTICK:     pad.cycleCamera = true; break;
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
                if      (event.caxis.value < -8192) { pad.left  = true;  pad.right = false; }
                else if (event.caxis.value >  8192) { pad.right = true;  pad.left  = false; }
                else                                { pad.left  = false; pad.right = false; }
                break;
            case SDL_CONTROLLER_AXIS_LEFTY:
                if      (event.caxis.value < -8192) { pad.up   = true;  pad.down  = false; }
                else if (event.caxis.value >  8192) { pad.down = true;  pad.up    = false; }
                else                                { pad.up   = false; pad.down  = false; }
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
// Velocidad
// ----------------------------------------------------------------------------

void System::setFastSpeed()   { minimumFrameTime = SCROLL_FRAME_TIME; }
void System::setNormalSpeed() { minimumFrameTime = GAME_FRAME_TIME;   }
//void System::setFastSpeed()   { minimumFrameTime = 1; }
//void System::setNormalSpeed() { minimumFrameTime = 1;   }

// ----------------------------------------------------------------------------
// Utilidades
// ----------------------------------------------------------------------------

void System::exitGame() { exit = true; }

void System::print(const std::string message)
{
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_DEBUG, "ABBEY", "%s\n", message.c_str());
#else
    std::cout << message;
#endif
}


