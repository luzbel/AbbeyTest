#ifndef _SDL_PALETA_H_
#define _SDL_PALETA_H_

#include "Types.h"

class Paleta {
public:
	Paleta(UINT8* gfxVGA) { _paletas=gfxVGA; };
	~Paleta();

	void setGamePalette(int pal, const SDL_PixelFormat * format ) {
		assert((pal >= 0) && (pal < 6)); // TODO, quitar valor a fuego
		if (pal==0) {
			for (int i = 0; i < 256; i++){
				setColor(i,0,0,0,format);
			}
		}
		else{
			UINT8 *p=_paletas;
			p+=_despPaletasVGA[pal];
			for (int i = 0; i < 256; i++){
				UINT8 r=*p++;
				UINT8 g=*p++;
				UINT8 b=*p++;
				[[maybe_unused]]UINT8 a=*p++;
				setColor(i,r,g,b,format);
			}
		}
	}

	void setColor(UINT8 index,UINT8 red,UINT8 green,UINT8 blue, const SDL_PixelFormat * format) {
		assert((index >= 0) && (index < 256)); // TODO, quitar valor a fuego
		rgb[index]=SDL_MapRGB(format,red,green,blue);
	};
private:
	UINT8 *_paletas; // Datos de la paleta en la ROM original
	INT32 _despPaletasVGA[6] = {
		-1,		// paleta negra , en realidad no la tenemos en fichero y la generamos en codigo
		0xC00,		// paleta del pergamino TODO: FALTA POR COMPROBAR QUE ESTA PALETA ES PARA EL PERGAMINO
		0,		// paleta de día durante el juego
		0x400,		// paleta de noche durante el juego
		0x800,		// paleta lámpara TODO: FALTA HACER CAMBIOS EN EL CODIGO PARA QUE LA USE
		0x1000		// paleta introducción
	};
public:
	UINT32 rgb[255];
};

#endif	// _SDL_PALETA_H
