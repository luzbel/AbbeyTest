#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

void System::updateScreen()
{
#ifdef __EMSCRIPTEN__
    if (interruptCounter % 6 == 0) {
#endif

//TODO si no está en modo webGL igual se puede simplificar
	if((bool)sys->useWebGL) {

//		SDL_Log("updateScreen %d\n", (int)_state);
		switch(_state)
		{
			case Abadia::STATES::INTRO:
				SDL_UpdateTexture(texturePR, nullptr, surfaceIntro->pixels, surface->pitch);
//				SDL_UpdateTexture(textureNPR, nullptr, surfaceMenu->pixels, surface->pitch);
				break;
			case Abadia::STATES::CONFIG_GFX:
			case Abadia::STATES::CONFIG_SND:
			case Abadia::STATES::HELP:
			case Abadia::STATES::HELP_MANEJO:
			case Abadia::STATES::HELP_AYUDAS:
			case Abadia::STATES::HELP_CAMARAS:
			case Abadia::STATES::CONFIG:
			case Abadia::STATES::ASK_EXIT:
			case Abadia::STATES::ASK_CONTINUE:
			case Abadia::STATES::ASK_NEW_GAME:
			case Abadia::STATES::LANGUAGE:
			case Abadia::STATES::LOAD:
			case Abadia::STATES::SAVE:
				SDL_UpdateTexture(texturePR, nullptr, surfaceIntro->pixels, surface->pitch);
				SDL_UpdateTexture(textureNPR, nullptr, surfaceMenu->pixels, surface->pitch);
				SDL_UpdateTexture(textureNPL, nullptr, surfaceMap->pixels, surface->pitch);
				break;
			case Abadia::STATES::MENU:
				// ojo, que esto se pinta como si fuese el final del flip
				// de la portada. Por eso portada es PR (aunque girada)
				// y NPR es el menu
				SDL_UpdateTexture(texturePR, nullptr, surfaceIntro->pixels, surface->pitch);
				SDL_UpdateTexture(textureNPR, nullptr, surfaceMenu->pixels, surface->pitch);
				SDL_UpdateTexture(textureNPL, nullptr, surfaceMap->pixels, surface->pitch);
				break;
			case Abadia::STATES::SCROLL:
			case Abadia::STATES::HELP_MANEJO_PERGAMINO:
			case Abadia::STATES::HELP_REFERENCIAS:   // referencias es un pergamino, no texto por marcador
			case Abadia::STATES::HELP_INTRODUCCION:
			case Abadia::STATES::ENDING:
				SDL_UpdateTexture(texturePR, nullptr, surfaceMenu->pixels, surface->pitch);
				SDL_UpdateTexture(textureNPR, nullptr, surface->pixels, surface->pitch);
				break;
			case Abadia::STATES::PLAY:
				SDL_UpdateTexture(texturePR, nullptr, surface->pixels, surface->pitch);
				SDL_UpdateTexture(textureNPR, nullptr, surface->pixels, surface->pitch);
				SDL_UpdateTexture(textureNPL, nullptr, surfaceMap->pixels, surface->pitch);
				break;
		}
    } else
	SDL_UpdateTexture(texturePR, nullptr, surface->pixels, surface->pitch);

    SDL_SetRenderTarget(renderer, nullptr);
    SDL_RenderClear(renderer);

if (useWebGL && shaderProgramBook && shaderProgramPage && shaderProgramPage) {


#ifdef __EMSCRIPTEN__
	// ver si esto se puede captar en handleEvents
	// de alguna manera para no hacerlo constantemente
	// en updateScreen
    int canvasW, canvasH;
EMSCRIPTEN_RESULT r =     emscripten_get_canvas_element_size("#canvas", &canvasW, &canvasH);
    SDL_Log("r: %d deberia ser %d - canvas %d x %d\n",r,EMSCRIPTEN_RESULT_SUCCESS, canvasW, canvasH);
    //if (canvasW != w || canvasH != h) {
    if (canvasW != 0 && canvasH != 0) {
        SDL_SetWindowSize(window, canvasW, canvasH);
        w = canvasW;
        h = canvasH;
    }
#endif

    // ver si esto se puede garantizar que nos llega en handleEvents
    // y actualizamos el tamaño solo cuando cambie la ventana (o el canvas en emscripten)
    int ww, wh;
    SDL_GetWindowSize(window, &ww, &wh);
    glViewport(0, 0, ww, wh);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Vincular las 4 texturas a sus unidades GL
    float tw, th;
    glActiveTexture(GL_TEXTURE0); SDL_GL_BindTexture(texturePR,      &tw, &th);
    glActiveTexture(GL_TEXTURE1); SDL_GL_BindTexture(texturePL,  &tw, &th);
    glActiveTexture(GL_TEXTURE2); SDL_GL_BindTexture(textureNPR,   &tw, &th);
    glActiveTexture(GL_TEXTURE3); SDL_GL_BindTexture(textureNPL, &tw, &th);

    GLint oldProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

    // Helper lambda para pasar uniforms comunes a cualquier programa
    auto setCommonUniforms = [&](GLuint prog) {
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTexturePR"),      0);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTexturePL"),  1);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTextureNPR"),   2);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTextureNPL"), 3);
        _gl_Uniform1f(_gl_GetUniformLocation(prog, "uFlipT"),        (float)uFlipT);
        _gl_Uniform1f(_gl_GetUniformLocation(prog, "uEfecto"),       (float)paletaEfecto);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uFiltro"),       (int)filtro);
        _gl_Uniform2f(_gl_GetUniformLocation(prog, "uTexSize"),      (float)TEXTURE_WIDTH, (float)TEXTURE_HEIGHT);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uPortada"),       (int)(
		(_state==Abadia::STATES::INTRO)||
		(_state==Abadia::STATES::MENU && (uFlipT<1.0))
		)?1:0);
    };

    // Helper lambda para dibujar un quad
    auto drawQuad = [&](GLuint prog, GLfloat* verts, GLfloat* uvs, int count) {
        GLint posLoc = _gl_GetAttribLocation(prog, "aPosition");
        GLint uvLoc  = _gl_GetAttribLocation(prog, "aTexCoord");
        GLuint vbo[2];
        _gl_GenBuffers(2, vbo);
        _gl_BindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        _gl_BufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(GLfloat), verts, GL_STREAM_DRAW);
        _gl_EnableVertexAttribArray(posLoc);
        _gl_VertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        _gl_BindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        _gl_BufferData(GL_ARRAY_BUFFER, count * 2 * sizeof(GLfloat), uvs, GL_STREAM_DRAW);
        _gl_EnableVertexAttribArray(uvLoc);
        _gl_VertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
        _gl_DisableVertexAttribArray(posLoc);
        _gl_DisableVertexAttribArray(uvLoc);
        _gl_DeleteBuffers(2, vbo);
    };

    GLfloat uvs[] = { 0.f,0.f, 1.f,0.f, 0.f,1.f, 1.f,1.f };

    // Avanzar animación
    if (uFlipInProgress==true) {
	    uFlipT += 0.01f;
	    if (uFlipT > 1.0f) {
		    uFlipInProgress=false;
	    }
    }
   
    // === Draw 1: páginas fijas (izquierda + derecha) ===
  /*  
    GLfloat vertsPage[] = {
        -1.f,  1.f,
         1.f,  1.f,
        -1.f, -1.f,
         1.f, -1.f
    }; */
/* solo para la izquierda
    GLfloat vertsPage[] = {
    0.0f,  0.9f,
    0.9f,  0.9f,
    0.0f, -0.9f,
    0.9f, -0.9f */
    GLfloat vertsPage[] = {
    -0.9f,  0.9f,
     0.9f,  0.9f,
    -0.9f, -0.9f,
     0.9f, -0.9f
};

//if ( (_state == Abadia::STATES::INTRO || _state==Abadia::STATES::MENU)  && uFlipT < 1.0f) 
//if ( _state != Abadia::STATES::INTRO ) 
if (true)
{
//	SDL_Log("Abadia::STATES::INTRO && uFlipT < 1.0f\n");

    _gl_UseProgram(shaderProgramPage);
    setCommonUniforms(shaderProgramPage);
    drawQuad(shaderProgramPage, vertsPage, uvs, 4);
}
//if (_state==Abadia::STATES::INTRO || _state==Abadia::STATES::MENU) {
if (true)
{
    // === Draw 2: tapa con vertex shader de rotación ===
 /*   
    GLfloat vertsCover[] = {
        0.0f,  1.f,
        1.0f,  1.f,
        0.0f, -1.f,
        1.0f, -1.f
    }; */
    GLfloat vertsCover[] = {
    0.0f,  0.9f,
    0.9f,  0.9f,
    0.0f, -0.9f,
    0.9f, -0.9f 
};

// if (uFlipT > 0.0f) 
	{
//	SDL_Log("Abadia::STATES::INTRO && uFlipT < 1.0f -> uFlipT > 0.0f\n");
    _gl_UseProgram(shaderProgramCover);
    setCommonUniforms(shaderProgramCover);
    drawQuad(shaderProgramCover, vertsCover, uvs, 4); 
	}

}

/*
if (true ) {
// Activar scissor para limpiar solo la mitad izquierda
glEnable(GL_SCISSOR_TEST);
glScissor(0, 0, ww/2, wh);
glClearColor(0.149f, 0.127f, 0.361f, 1.f);  // color fondo
glClear(GL_COLOR_BUFFER_BIT);
glDisable(GL_SCISSOR_TEST);
}
*/


    // Desvincular texturas
    glActiveTexture(GL_TEXTURE3); SDL_GL_UnbindTexture(textureNPL);
    glActiveTexture(GL_TEXTURE2); SDL_GL_UnbindTexture(textureNPR);
    glActiveTexture(GL_TEXTURE1); SDL_GL_UnbindTexture(texturePL);
    glActiveTexture(GL_TEXTURE0); SDL_GL_UnbindTexture(texturePR);

    SDL_GL_SwapWindow(window);
    _gl_UseProgram(oldProgram);
} else {
#ifdef ANDROID
    SDL_RenderCopy(renderer, texturePR, nullptr, nullptr);
#else
    SDL_RenderCopy(renderer, texturePR, nullptr, &dstrect);
#endif
    SDL_RenderPresent(renderer);
}

#ifdef __EMSCRIPTEN__
    }
#endif
}

