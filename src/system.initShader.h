void System::initShader(int efectoPaleta)
{
	SDL_Log("initShader 1\n");
    if (!useWebGL) return;
	SDL_Log("initShader 2\n");
    const char* vertexSource = R"(
// vertex shader de la tapa
attribute vec2 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
uniform float uFlipT;

void main() {
    vTexCoord = aTexCoord;
    vec2 pos = aPosition;
    // Rotar sobre x=0.0 según uFlipT
    // El borde libre (aTexCoord.x=1) se mueve, el lomo (aTexCoord.x=0) no
    float angle = uFlipT * 3.14159;
    pos.x = aPosition.x * cos(angle);  // proyección perspectiva simple
    gl_Position = vec4(pos, 0.0, 1.0);
}
    )";

    const char* vertexSimpleSource = R"(
attribute vec2 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    vTexCoord = aTexCoord;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";

	std::string fragmentSource = 
		std::string("#define PAGE_LEFT ") + "uTextureMenu" + "\n" + 
		std::string("#define PAGE_RIGHT ") + "uTextureIntro" + "\n" +
		std::string("#define NEXT_PAGE_LEFT ") + "uTextureMap" + "\n" + 
		std::string("#define NEXT_PAGE_RIGHT ") + "uTexture" + "\n" +
#include "shader_common.glsl"
#include "shader_xbr.glsl"
#include "shader_book_cover.glsl"
#include "shader_main.glsl"

    std::cout << "DEBUG SHADER CONTENT:\n" << fragmentSource << "\n---END---" << std::endl;

	std::string shaderProgramPage =
		std::string("#define PAGE_LEFT ") + "uTextureMenu" + "\n" + 
		std::string("#define PAGE_RIGHT ") + "uTextureIntro" + "\n" +
		std::string("#define NEXT_PAGE_LEFT ") + "uTextureMap" + "\n" + 
		std::string("#define NEXT_PAGE_RIGHT ") + "uTexture" + "\n" +
#include "shader_common.glsl" 
#include "shader_xbr.glsl"
#include "shaderProgramPage.glsl"	


    auto compileShader = [&](GLenum type, const char* src) -> GLuint {
        GLuint s = _gl_CreateShader(type);
        _gl_ShaderSource(s, 1, &src, nullptr);
        _gl_CompileShader(s);
        return s;
    };

    GLuint vert = compileShader(GL_VERTEX_SHADER,   vertexSource);
    GLuint vert2 = compileShader(GL_VERTEX_SHADER,   vertexSimpleSource);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
    GLuint frag2 = compileShader(GL_FRAGMENT_SHADER, shaderProgramPage.c_str());

    shaderProgram = _gl_CreateProgram();
    _gl_AttachShader(shaderProgram, vert);
    _gl_AttachShader(shaderProgram, frag);
    _gl_LinkProgram(shaderProgram);

    shaderProgram2 = _gl_CreateProgram();
    _gl_AttachShader(shaderProgram2, vert2);
    _gl_AttachShader(shaderProgram2, frag2);
    _gl_LinkProgram(shaderProgram2);

std::string vertexCoverSource=
#include "vertexCoverSource"


std::string fragCoverSource =
		std::string("#define PAGE_LEFT ") + "uTextureMenu" + "\n" + 
		std::string("#define PAGE_RIGHT ") + "uTextureIntro" + "\n" +
		std::string("#define NEXT_PAGE_LEFT ") + "uTextureMap" + "\n" + 
		std::string("#define NEXT_PAGE_RIGHT ") + "uTexture" + "\n" +
#include "shader_common.glsl"
#include "shader_xbr.glsl"
#include "shaderProgramPageXXX.glsl"	

GLuint vertCover = compileShader(GL_VERTEX_SHADER,   vertexCoverSource.c_str());
GLuint fragCover = compileShader(GL_FRAGMENT_SHADER, fragCoverSource.c_str());
shaderProgramCover = _gl_CreateProgram();
_gl_AttachShader(shaderProgramCover, vertCover);
_gl_AttachShader(shaderProgramCover, fragCover);
_gl_LinkProgram(shaderProgramCover);

        GLint status;
    _gl_GetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    SDL_Log("status1 %d\n",status);
    if (status == GL_FALSE) {
    SDL_Log("**********\n************\nCAGADA\n**********\n********\nstatus1 %d\n",status);
     //   char log[512]; _gl_GetProgramInfoLog(shaderProgram, sizeof(log), nullptr, log);
//        SDL_Log("ERROR ENLACE SHADER 1: %s", log);
        useWebGL = false; return;
    }
    SDL_Log("status2 %d\n",status);
        GLint status2;
    _gl_GetProgramiv(shaderProgram2, GL_LINK_STATUS, &status2);
    SDL_Log("status1 de 2 %d\n",status2);
 
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
