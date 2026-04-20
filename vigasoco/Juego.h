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
public:		
	int idioma;  // idioma de los textos TODO ponerlo como un enumerado
	// GraficosCPC estaba como privado
	// se pone como publico para que pueda acceder
	// el MotorGrafico.cpp y decidir si tiene que parchear las pantallas
	// o no
	// TODO: Poner metodos get para no dar acceso directo y que lo 
	// pueda cambiar por error desde otro lado
	bool GraficosCPC; // Indica si se usan los datos del archivo GraficosCPC
			  // o del GraficosVGA
			  // En ambos casos, son de 8 bits
	
	UINT8 buffer[8192*2];			// buffer para mezclar los sprites y para buscar las rutas
	UINT8 *roms;				// puntero a las roms originales
	Logica *logica;				// objeto que se encarga de gestionar la l??gica del juego
	
	Pergamino *pergamino;			// pergamino para la presentación y el final
	Marcador *marcador;			// marcador del juego
	MotorGrafico *motor;			// motor gráfico

	Sprite *sprites[numSprites];		// sprites del juego
	Puerta *puertas[numPuertas];		// puertas del juego
	Objeto *objetos[numObjetos];		// objetos del juego
	Personaje *personajes[numPersonajes];	// personajes del juego

	bool pausa;				// indica si el juego está pausado
	bool modoInformacion;			// modo de información del juego
	bool cambioModoInformacion; 		// se ha cambiado el estado
	InfoJuego *infoJuego;			// objeto para mostrar información interna del juego
	Abadia::STATES currentState;
	//int firstTime;	
	int seleccionado;
	ConfigReader *configReader;
	int selectedSlot;


private:
	bool cargar(int slot);
	void save(int slot);
	
    /////////////////////////////////////////////////////////
	void pintaMenuCargar(int seleccionado,bool efecto=false);

	bool menuCargar2();
	void pintaMenuGrabar(int seleccionado,bool efecto=false);
	bool menuGrabar2();
	bool menuIntroduccion(void);
	void pintaMenuTeclado(int seleccionado);
	bool menuTeclado(void);
	void pintaMenuCamaras(int seleccionado);
	bool menuCamaras(void);
	void pintaMenuMejoras(int seleccionado);
	bool menuMejoras(void);
	void pintaMenuTutorial(int seleccionado,bool efecto=false);
	bool menuTutorial(void);
	void pintaMenuAyuda(int seleccionado,bool efecto=false);
	bool menuAyuda(void);
	void pintaMenuIdioma(int seleccionado,bool efecto=false);
	bool menuIdioma(void);
	void pintaMenuPrincipal(int seleccionado,bool efecto=false);
	bool menu(void);
    ////////////////////////////////////////////////////////////
    
	void cambioCPC_VGA(void);
	void compruebaCambioCPC_VGA(void);
	bool compruebaMenu(void);
//	void ReiniciaPantalla(void); // lo ponemos publico para que cada vez que se cambie la paleta se reinicie
	// es muy abadia, y muy cpc 
	// pero no tiene sentido tenerlo en una clase sola
	// ni en system
	// pixel packing
	inline int packPixelMode1(int oldByte, int pixel, int color)
	{
		assert ((pixel >= 0) && (pixel < 4));
		assert ((color >= 0) && (color < 4));

		// find out the 2 bits of the new pixel
		int mask = 0x88;
		mask = mask >> pixel;

		// save the other pixels
		oldByte = (oldByte & (~mask)) & 0xff;

		// array with the four colors
		static int byteColors[4] = { 0x00, 0xf0, 0x0f, 0xff };

		// combines the other pixels with the new pixel
		return oldByte | (byteColors[color] & mask);
	}

public:
	void ReiniciaPantalla(bool mostrarDiaYMomentoDia=true); // lo ponemos publico para que cada vez que se cambie la paleta se reinicie
	// es muy abadia, y muy cpc 
	// pero no tiene sentido tenerlo en una clase sola
	// ni en system
	inline int unpackPixelMode1(int data, int pixel)
	{
		return (((data >> (3 - pixel)) & 0x01) << 1) | ((data >> (7 - pixel)) & 0x01);
	}


	void muestraFinal();
	void limpiaAreaJuego(int color);

	// bucle principal del juego
	void preRun();
	void run();
	void run2();
	void stateMachine();
	void changeState(Abadia::STATES newState);

	// inicialización y limpieza
	Juego(UINT8 *romData);
	~Juego();
	bool showingMenu;
	bool activeGame;

protected:
	void muestraPresentacion();
	void muestraIntroduccion();
	bool muestraPantallaFinInvestigacion();

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
	
	void menuAsk();	
	void askForNewGame();
	void askForNewGameLogic();

	void askToContinue();
	void askToContinueLogic();

	void askExit();
	void askExitLogic();

};

}

#endif	// _ABADIA_JUEGO_H_
