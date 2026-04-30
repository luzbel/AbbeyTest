// Juego.h
//
//		Clase principal del juego. Almacena el estado y las entidades del juego.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ABADIA_JUEGO_H__
#define __ABADIA_JUEGO_H__

#include <stdio.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <sys/stat.h> 

#include "Singleton.h"
#include "Types.h"
#include "configreader.h"

#include "SimpleMenu.h"

#include "system.h"

namespace Abadia {

class InfoJuego;				// definido en InfoJuego.h
class Logica;					// definido en Logica.h
class Marcador;					// definido en Marcador.h
class MotorGrafico;				// definido en MotorGrafico.h
class Objeto;					// definido en Objeto.h
class Pergamino;				// definido en Pergamino.h
class Personaje;				// definido en Personaje.h
class Puerta;					// definido en Puerta.h
class Sprite;					// definido en Sprite.h

#define elJuego Juego::getSingletonPtr()

class Juego : public Singleton<Juego>
{
// constantes
public:
	static const int numPersonajes = 8;
	static const int numPuertas = 7;
	static const int numObjetos = 8;

	static const int primerSpritePersonajes = 0;
	static const int primerSpritePuertas = primerSpritePersonajes + numPersonajes;
	static const int primerSpriteObjetos = primerSpritePuertas + numPuertas;
	static const int spritesReflejos = primerSpriteObjetos + numObjetos;
	static const int spriteLuz = spritesReflejos + 2;
	static const int numSprites = spriteLuz + 1;
	static const char *savefile[7];
	
// campos
private:
	bool mute;
	unsigned short slot;
	bool pausaPorEstarEnMenus;
	SimpleMenu mainMenu;
	//bool menuInitialized=false;
	bool mainMenuReady=false;
public:		
	int idioma;
	bool GraficosCPC;
	
	UINT8 buffer[8192*2];
	UINT8 *roms;
	Logica *logica;
	
	Pergamino *pergamino;
	Marcador *marcador;
	MotorGrafico *motor;

	Sprite *sprites[numSprites];
	Puerta *puertas[numPuertas];
	Objeto *objetos[numObjetos];
	Personaje *personajes[numPersonajes];

	bool modoInformacion;
	bool cambioModoInformacion;
	InfoJuego *infoJuego;
	Abadia::STATES currentState;
	int seleccionado;
	ConfigReader *configReader;
	int selectedSlot;
	bool showingMenu;
	bool activeGame;

public:
	// inicialización y limpieza
	Juego(UINT8 *romData);
	~Juego();

	// bucle principal del juego
	void preRun();
	void run();
	void changeState(Abadia::STATES newState);
	void ReiniciaPantalla(bool mostrarDiaYMomentoDia=true);
	void limpiaAreaJuego(int color);

	// estados gestionados desde Abbey
	void muestraPresentacion();
	void muestraIntroduccion();
	void muestraFinal();
	bool muestraPantallaFinInvestigacion();
	bool menu();
	bool menuCargar2();
	bool menuGrabar2();
	bool menuIdioma();
	void askForNewGame();
	void askToContinue();
	void askExit();

	bool estaPausado() { return pausaPorEstarEnMenus; };

	inline int unpackPixelMode1(int data, int pixel)
	{
		return (((data >> (3 - pixel)) & 0x01) << 1) | ((data >> (7 - pixel)) & 0x01);
	}

protected:
	void creaEntidadesJuego();
	void actualizaLuz();
	void generaGraficosFlipeados();
	void flipeaGraficos(UINT8 *tablaFlip, UINT8 *src, UINT8 *dest, int ancho, int bytes);
	void generaGraficosFlipeadosVGA();
	void flipeaGraficosVGA(UINT8 *src, UINT8 *dest, int ancho, int bytes);
	
	void checkForSaveFiles();
	bool saveFileExist[7];
	string saveFile[7];
	void checkConfigFile();
	bool readConfigFile();
	bool saveConfigFile();
	string getDateAndTime();

	void askForNewGameLogic();
	void askToContinueLogic();
	void askExitLogic();

	void pintaMenuCargar(int seleccionado, bool efecto=false);
	void pintaMenuGrabar(int seleccionado, bool efecto=false);
	void pintaMenuIdioma(int seleccionado, bool efecto=false);
//	void pintaMenuPrincipal(int seleccionado, bool efecto=false);

	bool cargar(int slot);
	void save(int slot);

	void cambioCPC_VGA();

	inline int packPixelMode1(int oldByte, int pixel, int color)
	{
		assert ((pixel >= 0) && (pixel < 4));
		assert ((color >= 0) && (color < 4));

		int mask = 0x88;
		mask = mask >> pixel;
		oldByte = (oldByte & (~mask)) & 0xff;
		static int byteColors[4] = { 0x00, 0xf0, 0x0f, 0xff };
		return oldByte | (byteColors[color] & mask);
	}
};

}

#endif	// _ABADIA_JUEGO_H_
