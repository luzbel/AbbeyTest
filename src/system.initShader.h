void System::initShader(int efectoPaleta)
{
    if (!useWebGL) return;

    // ---- Vertex shaders ----

    // Quad plano sin transformación — para páginas estáticas
    const char* vertexSimpleSource = R"(
#ifdef GL_ES
precision mediump float;
#endif
attribute vec2 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";

    // Quad con rotación perspectiva sobre el eje izquierdo — para la tapa
    // TODO: leer de fichero vertexCoverSource
    const char* vertexCoverSource = R"(
#ifdef GL_ES
precision mediump float;
#endif
attribute vec2 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
uniform float uFlipT;
void main() {
    vTexCoord = aTexCoord;
    float angle = uFlipT * 3.14159;
    float cosA  = cos(angle);
	// Jugar con el 0.85 del max(abs(cosA), XXX) 
	// para que la perspectiva del libro al abrir sea mayor o menor
    float w     = mix(1.0, max(abs(cosA), 0.85), aTexCoord.x);
    gl_Position = vec4(aPosition.x * cosA, aPosition.y, 0.0, w);
}
)";

    // ---- Defines comunes de texturas ----
    std::string textureDefines =
        std::string("#define PAGE_RIGHT ")      + "uTexturePR" + "\n" +
        std::string("#define PAGE_LEFT ")       + "uTexturePL"  + "\n" +
        std::string("#define NEXT_PAGE_RIGHT ") + "uTextureNPR"      + "\n"+
        std::string("#define NEXT_PAGE_LEFT ")  + "uTextureNPL"   + "\n";

    // ---- Fragment shaders ----

    // Libro abierto estático — reservado para uso futuro
    std::string fragBook =
        textureDefines +
#include "shader_common.glsl"
#include "shader_xbr.glsl"
#include "shader_book_cover.glsl"
#include "shader_main.glsl"

    // Página fija (derecha) — quad plano con NEXT_PAGE_RIGHT
    std::string fragPage =
        textureDefines +
#include "shader_common.glsl"
#include "shader_xbr.glsl"
#include "frag_page.glsl"

    // Tapa girando — anverso PAGE_RIGHT, reverso NEXT_PAGE_LEFT
    std::string fragCover =
        textureDefines +
#include "shader_common.glsl"
#include "shader_xbr.glsl"
#include "frag_cover.glsl"

    // ---- Compilación ----
    auto compileShader = [&](GLenum type, const char* src) -> GLuint {
        GLuint s = _gl_CreateShader(type);
        _gl_ShaderSource(s, 1, &src, nullptr);
        _gl_CompileShader(s); 
#ifdef __EMSCRIPTEN__
        GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512]; glGetShaderInfoLog(s, sizeof(log), nullptr, log);
            SDL_Log("ERROR compile shader: %s", log);
	}
#else
	    // en escritorio faltan los punteros a estas funciones
//		SDL_Log("ERROR compilando shader\n");
#endif
        return s;
    };

    auto linkProgram = [&](GLuint vert, GLuint frag) -> GLuint {
        GLuint prog = _gl_CreateProgram();
        _gl_AttachShader(prog, vert);
        _gl_AttachShader(prog, frag);
        _gl_LinkProgram(prog);
        GLint ok; _gl_GetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
#ifdef __EMSCRIPTEN__

            char log[512]; glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
            SDL_Log("ERROR link program: %s", log); 
#else
		SDL_Log("ERROR linkando shader\n");
#endif
        }
        return prog;
    };

SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"compile vertexSimpleSource\n");
SDL_Log("compile vertexSimpleSource\n");
    GLuint vertSimple = compileShader(GL_VERTEX_SHADER,   vertexSimpleSource);
SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"compile vertexCoverSource\n");
SDL_Log("compile vertexCoverSource\n");
    GLuint vertCover  = compileShader(GL_VERTEX_SHADER,   vertexCoverSource);
SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"compile fragBook\n");
SDL_Log("compile fragBook\n");
    GLuint fragBookC  = compileShader(GL_FRAGMENT_SHADER, fragBook.c_str());
SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"compile fragPage\n");
SDL_Log("compile fragPage\n");
    GLuint fragPageC  = compileShader(GL_FRAGMENT_SHADER, fragPage.c_str());

SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"compile fragCover\n");
SDL_Log("compile fragCover\n");
    GLuint fragCoverC = compileShader(GL_FRAGMENT_SHADER, fragCover.c_str());

SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"link fragBookC\n");
SDL_Log("link fragBookC\n");
    shaderProgramBook  = linkProgram(vertSimple, fragBookC);   // futuro: libro abierto
SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"link fragPageC\n");
SDL_Log("link fragPageC\n");
    shaderProgramPage  = linkProgram(vertSimple, fragPageC);   // página fija
SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION,"link fragCoverC\n");
SDL_Log("link fragCoverC\n");
    shaderProgramCover = linkProgram(vertCover,  fragCoverC);  // tapa girando

    if (!shaderProgramBook || !shaderProgramPage || !shaderProgramCover) {
        useWebGL = false;
        return;
    }

    // Locations de shaderProgramBook (usado también como referencia) 
    /*
    efectoLocation       = _gl_GetUniformLocation(shaderProgramBook, "uEfecto");
    filtroLocation       = _gl_GetUniformLocation(shaderProgramBook, "uFiltro");
    texSizeLocation      = _gl_GetUniformLocation(shaderProgramBook, "uTexSize");
    texturePRLocation      = _gl_GetUniformLocation(shaderProgramBook, "uTexturePR");
    texturePLLocation   = _gl_GetUniformLocation(shaderProgramBook, "uTexturePL");
    textureNPRLocation  = _gl_GetUniformLocation(shaderProgramBook, "uTextureNPR");
    textureNPLLocation = _gl_GetUniformLocation(shaderProgramBook, "uTextureNPL");
    flipTLocation        = _gl_GetUniformLocation(shaderProgramBook, "uFlipT");
    */

    SDL_Log("initShader OK — book=%u page=%u cover=%u",
            shaderProgramBook, shaderProgramPage, shaderProgramCover);
}
