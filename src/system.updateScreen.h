void System::updateScreen()
{
#ifdef __EMSCRIPTEN__
    if (interruptCounter % 6 == 0) {
#endif

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

if (useWebGL && shaderProgramBook) {
    int ww, wh;
    SDL_GetWindowSize(window, &ww, &wh);
    glViewport(0, 0, ww, wh);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Vincular las 4 texturas a sus unidades GL
    float tw, th;
    glActiveTexture(GL_TEXTURE0); SDL_GL_BindTexture(texture,      &tw, &th);
    glActiveTexture(GL_TEXTURE1); SDL_GL_BindTexture(textureMenu,  &tw, &th);
    glActiveTexture(GL_TEXTURE2); SDL_GL_BindTexture(textureMap,   &tw, &th);
    glActiveTexture(GL_TEXTURE3); SDL_GL_BindTexture(textureIntro, &tw, &th);
    glActiveTexture(GL_TEXTURE0);

    GLint oldProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

    // Helper lambda para pasar uniforms comunes a cualquier programa
    auto setCommonUniforms = [&](GLuint prog) {
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTexture"),      0);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTextureMenu"),  1);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTextureMap"),   2);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uTextureIntro"), 3);
        _gl_Uniform1f(_gl_GetUniformLocation(prog, "uFlipT"),        (float)uFlipT);
        _gl_Uniform1f(_gl_GetUniformLocation(prog, "uEfecto"),       (float)paletaEfecto);
        _gl_Uniform1i(_gl_GetUniformLocation(prog, "uFiltro"),       (int)filtro);
        _gl_Uniform2f(_gl_GetUniformLocation(prog, "uTexSize"),      (float)TEXTURE_WIDTH, (float)TEXTURE_HEIGHT);
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
    uFlipT += 0.01f;
    if (uFlipT > 1.0f) uFlipT = 1.0f;

    // === Draw 1: páginas fijas (izquierda + derecha) ===
  /*  
    GLfloat vertsPage[] = {
        -1.f,  1.f,
         1.f,  1.f,
        -1.f, -1.f,
         1.f, -1.f
    }; */

    GLfloat vertsPage[] = {
    0.0f,  0.9f,
    0.9f,  0.9f,
    0.0f, -0.9f,
    0.9f, -0.9f
}; 
    _gl_UseProgram(shaderProgramPage);
    setCommonUniforms(shaderProgramPage);
    drawQuad(shaderProgramPage, vertsPage, uvs, 4);

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
    _gl_UseProgram(shaderProgramCover);
    setCommonUniforms(shaderProgramCover);
    drawQuad(shaderProgramCover, vertsCover, uvs, 4);

    // Desvincular texturas
    glActiveTexture(GL_TEXTURE3); SDL_GL_UnbindTexture(textureIntro);
    glActiveTexture(GL_TEXTURE2); SDL_GL_UnbindTexture(textureMap);
    glActiveTexture(GL_TEXTURE1); SDL_GL_UnbindTexture(textureMenu);
    glActiveTexture(GL_TEXTURE0); SDL_GL_UnbindTexture(texture);

    SDL_GL_SwapWindow(window);
    _gl_UseProgram(oldProgram);
} else {
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

