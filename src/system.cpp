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
#endif

// Estado GL
static SDL_GLContext _glContext    = nullptr;
static GLuint        _glTexture    = 0;
static GLuint        shaderProgram = 0;
static GLint         efectoLocation = -1;
static SDL_Window* _glWindow = nullptr;

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

    if (!_gl_CreateShader || !_gl_CreateProgram || !_gl_UseProgram) {
        return false;
    }
    return true;
}
#endif

// ----------------------------------------------------------------------------
// initShader
// Compila el fragment shader y crea la textura GL que recibirá los pixels.
// Llámalo después de loadConfig() y de que exista el contexto GL.
// ----------------------------------------------------------------------------

void System::initShader(int efectoPaleta)
{
    if (!useWebGL) return;

    // Vertex shader mínimo: quad NDC fullscreen
    /*
    const char* vertexSource = R"(
        attribute vec2 aPosition;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        void main() {
            vTexCoord   = aTexCoord;
            gl_Position = vec4(aPosition, 0.0, 1.0);
        }
    )"; 
    */





    // Fragment shader: paleta + efectos futuros (xbr, hqx, pageflip…) 
    /*
    const char* fragmentSource = R"(
        varying vec2 vTexCoord;
        uniform sampler2D uTexture;
        uniform int uEfecto;
        void main() {
            vec4 color = texture2D(uTexture, vTexCoord);
            float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));
            if      (uEfecto == 1) gl_FragColor = vec4(grey, grey, grey, color.a);
            else if (uEfecto == 2) gl_FragColor = vec4(grey * 0.2, grey * 0.9, grey * 0.1, color.a);
            else if (uEfecto == 3) gl_FragColor = vec4(grey * 0.9, grey * 0.5, 0.0, color.a);
            else                   gl_FragColor = color;
        }
    )"; */
/*
    const char* fragmentSource = R"(
    varying vec2 vTexCoord;
    uniform sampler2D uTexture;
    uniform int uEfecto;
    void main() {
        gl_FragColor = texture2D(uTexture, vTexCoord);
    }
)"; */
/*
    const char* vertexSource = R"(
    void main() {
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    }
)";

    
const char* fragmentSource = R"(
    uniform sampler2D uTexture;
    void main() {
        gl_FragColor = texture2D(uTexture, gl_TexCoord[0].xy);
    }
)"; */ 

    /* este solo va en desktop
    const char* vertexSource = R"(
    void main() {
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    }
)";

    const char* fragmentSource = R"(
    uniform sampler2D uTexture;
    uniform int uEfecto;
    void main() {
        vec4 color = texture2D(uTexture, gl_TexCoord[0].xy);
        float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        if      (uEfecto == 1) gl_FragColor = vec4(grey, grey, grey, color.a);
        else if (uEfecto == 2) gl_FragColor = vec4(grey * 0.2, grey * 0.9, grey * 0.1, color.a);
        else if (uEfecto == 3) gl_FragColor = vec4(grey * 0.9, grey * 0.5, 0.0, color.a);
        else                   gl_FragColor = color;
    }
)";  */

#ifdef __EMSCRIPTEN__
    const char* vertexSource = R"(
        attribute vec2 aPosition;
        attribute vec2 aTexCoord;
        varying vec2 vTexCoord;
        void main() {
            vTexCoord   = aTexCoord;
            gl_Position = vec4(aPosition, 0.0, 1.0);
        }
    )";
    const char* fragmentSource = R"(
        precision mediump float;
        varying vec2 vTexCoord;
        uniform sampler2D uTexture;
        uniform int uEfecto;
        void main() {
            vec4 color = texture2D(uTexture, vTexCoord);
            float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));
            if      (uEfecto == 1) gl_FragColor = vec4(grey, grey, grey, color.a);
            else if (uEfecto == 2) gl_FragColor = vec4(grey * 0.2, grey * 0.9, grey * 0.1, color.a);
            else if (uEfecto == 3) gl_FragColor = vec4(grey * 0.9, grey * 0.5, 0.0, color.a);
            else                   gl_FragColor = color;
        }
    )";
#else
    const char* vertexSource = R"(
        void main() {
            gl_TexCoord[0] = gl_MultiTexCoord0;
            gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
        }
    )";
    const char* fragmentSource = R"(
        uniform sampler2D uTexture;
        uniform int uEfecto;
        void main() {
            vec4 color = texture2D(uTexture, gl_TexCoord[0].xy);
            float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));
            if      (uEfecto == 1) gl_FragColor = vec4(grey, grey, grey, color.a);
            else if (uEfecto == 2) gl_FragColor = vec4(grey * 0.2, grey * 0.9, grey * 0.1, color.a);
            else if (uEfecto == 3) gl_FragColor = vec4(grey * 0.9, grey * 0.5, 0.0, color.a);
            else                   gl_FragColor = color;
        }
    )";
#endif

    auto compileShader = [&](GLenum type, const char* src) -> GLuint {
        GLuint s = _gl_CreateShader(type);
        _gl_ShaderSource(s, 1, &src, nullptr);
        _gl_CompileShader(s);
        return s;
    };

    GLuint vert = compileShader(GL_VERTEX_SHADER,   vertexSource);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    shaderProgram = _gl_CreateProgram();
    _gl_AttachShader(shaderProgram, vert);
    _gl_AttachShader(shaderProgram, frag);
    _gl_LinkProgram(shaderProgram);
    _gl_UseProgram(shaderProgram);

    efectoLocation = _gl_GetUniformLocation(shaderProgram, "uEfecto");
    _gl_Uniform1i(efectoLocation, efectoPaleta);

    // Textura GL que recibirá los pixels de surface en cada frame
    glGenTextures(1, &_glTexture);
    glBindTexture(GL_TEXTURE_2D, _glTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Reservar espacio inicial
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 TEXTURE_WIDTH, TEXTURE_HEIGHT, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//    glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, 
 //                TEXTURE_WIDTH, TEXTURE_HEIGHT, GL_BGRA,
  //               , GL_UNSIGNED_BYTE, surface->pixels);
SDL_Log("initShader: _glTexture=%u efectoLocation=%d", _glTexture, efectoLocation);
    SDL_Log("initShader OK — shaderProgram=%u glTexture=%u", shaderProgram, _glTexture);
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
    // Contexto GL — siempre intentamos crearlo; si falla caemos a SW
    // -------------------------------------------------------------------
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    _glContext = SDL_GL_CreateContext(window);
#ifndef __EMSCRIPTEN__
    if (_glContext && !initGLPointers()) {
        print("WARNING: No se pudieron cargar punteros GL.\n");
        SDL_GL_DeleteContext(_glContext);
        _glContext = nullptr;
    }
#endif

	_glWindow = window;

    // -------------------------------------------------------------------
    // Surface — antes de loadConfig, los callbacks de paleta la necesitan
    // -------------------------------------------------------------------
    surface = SDL_CreateRGBSurface(0, TEXTURE_WIDTH, TEXTURE_HEIGHT, 32,
                                   rmask, gmask, bmask, amask);
    if (!surface) print("Error: Can't create surface.\n");
    _pixels       = static_cast<Uint32*>(surface->pixels);
    _pitch_pixels = surface->pitch / sizeof(UINT32);

    // -------------------------------------------------------------------
    // Callbacks antes de loadConfig para que se disparen al cargar
    // -------------------------------------------------------------------
    mute.setCallback([this](bool v) { setMute(v); });

    // -------------------------------------------------------------------
    // loadConfig — aquí se lee useWebGL real del fichero
    // -------------------------------------------------------------------
    loadConfig();

    // Si el contexto GL falló, forzamos SW independientemente del config
    if (!_glContext) useWebGL = false;

    // -------------------------------------------------------------------
    // Renderer y recursos de pantalla — DESPUÉS de loadConfig
    // -------------------------------------------------------------------
    if (useWebGL) {
        // Modo GL: renderer software mínimo por si algún subsistema lo usa
        //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	renderer=nullptr;
        initShader(paletaEfecto);
    } else {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    	if (!renderer) print("ERROR: Could not create renderer.\n");
        texture  = SDL_CreateTextureFromSurface(renderer, surface);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

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
    saveConfig();

    SDL_GameControllerClose(gamepad);
    SDL_HapticClose(hapticDevice);

    if (renderer)    SDL_DestroyRenderer(renderer);
    if (texture)     SDL_DestroyTexture(texture);
    if (_glContext)  SDL_GL_DeleteContext(_glContext);
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

    loadSlotDates();
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

// Modo GL: sube los pixels a la textura GL y dibuja un quad fullscreen.
// No usa SDL_Renderer para el blit principal, así el pipeline GL es nuestro.
static void renderGL(SDL_Surface* surface, GLuint glTexture,
                     GLuint program, GLint efectoLoc, int efecto,
                     SDL_Rect dstrect, int winW, int winH)
{
	SDL_GL_MakeCurrent(_glWindow, _glContext);
	SDL_Log("renderGL: surface=%p pixels=%p glTexture=%u program=%u efecto=%d",
        (void*)surface, surface ? surface->pixels : nullptr,
        glTexture, program, efecto);
    glBindTexture(GL_TEXTURE_2D, glTexture);
    /* 666
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    TEXTURE_WIDTH, TEXTURE_HEIGHT,
                    GL_BGRA, GL_UNSIGNED_BYTE,
                    surface->pixels); */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    TEXTURE_WIDTH, TEXTURE_HEIGHT,
                    GL_RGBA, GL_UNSIGNED_BYTE,
                    surface->pixels);

    _gl_UseProgram(program);
    _gl_Uniform1i(efectoLoc, efecto);

    glViewport(0, 0, winW, winH);
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Convertir dstrect (pixels) a NDC [-1,1]
    float x0 = ( 2.f * dstrect.x                  / winW) - 1.f;
    float x1 = ( 2.f * (dstrect.x + dstrect.w)    / winW) - 1.f;
    float y0 = 1.f - (2.f * dstrect.y             / winH);
    float y1 = 1.f - (2.f * (dstrect.y + dstrect.h) / winH);


    // Quad legacy (GL2 / GLES2 con gl_FragCoord disponible)
    // Para GLES2 puro habría que usar VBOs; en desktop GL2 glBegin está OK.
#ifndef __EMSCRIPTEN__
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
        glTexCoord2f(0.f, 0.f); glVertex2f(x0, y0);
        glTexCoord2f(1.f, 0.f); glVertex2f(x1, y0);
        glTexCoord2f(1.f, 1.f); glVertex2f(x1, y1);
        glTexCoord2f(0.f, 1.f); glVertex2f(x0, y1);
    glEnd();
#else
    // Emscripten/GLES2: usar atributos de vértice 
    /*
    GLfloat verts[] = { x0,y0, x1,y0, x0,y1, x1,y1 };
    GLfloat uvs[]   = { 0,0,   1,0,   0,1,   1,1   };
    GLint posLoc = glGetAttribLocation(program, "aPosition");
    GLint uvLoc  = glGetAttribLocation(program, "aTexCoord");
    glEnableVertexAttribArray(posLoc);
    glEnableVertexAttribArray(uvLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(uvLoc,  2, GL_FLOAT, GL_FALSE, 0, uvs);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(posLoc);
    glDisableVertexAttribArray(uvLoc); */
    GLfloat verts[] = { x0,y0,  x1,y0,  x0,y1,  x1,y1 };
    // imagen invertida boca abajo GLfloat uvs[]   = { 0,1,    1,1,    0,0,    1,0   };
    GLfloat uvs[]   = { 0,0,    1,0,    0,1,    1,1   };

    GLuint vbo[2];
    glGenBuffers(2, vbo);

    GLint posLoc = glGetAttribLocation(program, "aPosition");
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLint uvLoc = glGetAttribLocation(program, "aTexCoord");
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STREAM_DRAW);
    glEnableVertexAttribArray(uvLoc);
    glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(posLoc);
    glDisableVertexAttribArray(uvLoc);
    glDeleteBuffers(2, vbo);
#endif
}

void System::updateScreen()
{
#ifdef __EMSCRIPTEN__
    if (interruptCounter % 6 == 0) {
#endif

    if (useWebGL && _glContext && shaderProgram) {
        // --- Modo GL: blit directo, sin SDL_Renderer ---
        int ww, wh;
	SDL_GL_MakeCurrent(window, _glContext);
        SDL_GetWindowSize(window, &ww, &wh);
        renderGL(surface, _glTexture, shaderProgram, efectoLocation,
                 paletaEfecto, dstrect, ww, wh);
        SDL_GL_SwapWindow(window);
    } else {
        // --- Modo SW: pipeline SDL_Renderer clásico ---
        SDL_UpdateTexture(texture, nullptr, surface->pixels, surface->pitch);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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

// updateTexture se mantiene para compatibilidad con el código que la llame,
// pero en modo GL el upload real se hace en updateScreen.
void System::updateTexture()
{
    if (!useWebGL && texture)
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

// ----------------------------------------------------------------------------
// Utilidades
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
