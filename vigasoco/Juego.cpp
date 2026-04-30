///Juego.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include <string>

#include "Abad.h"
#include "Adso.h"
#include "Berengario.h"
#include "Bernardo.h"
#include "BuscadorRutas.h"

#include "GestorFrases.h"
#include "Guillermo.h"
#include "InfoJuego.h"
#include "Jorge.h"
#include "Juego.h"
#include "Logica.h"
#include "Malaquias.h"
#include "Marcador.h"
#include "Monje.h"
#include "MotorGrafico.h"
#include "Objeto.h"
#include "Pergamino.h"
#include "Personaje.h"
#include "PersonajeConIA.h"
#include "Puerta.h"
#include "RejillaPantalla.h"
#include "Severino.h"
#include "Sprite.h"
#include "SpriteLuz.h"
#include "SpriteMonje.h"

#include "Serializar.h"

#include <iostream>
#include <string.h>

#include "system.h"
#include "texts.h"

#define VITA_SAVE_DIR "ux0:data/Abbey/"

using namespace Abadia;

const char *Juego::savefile[7] = {	
	 "abadia0.save",
	 "abadia1.save",
	 "abadia2.save",
	 "abadia3.save",
	 "abadia4.save",
	 "abadia5.save",
	 "abadia6.save"
};


/////////////////////////////////////////////////////////////////////////////
// inicialización y limpieza
/////////////////////////////////////////////////////////////////////////////

Juego::Juego(UINT8 *romData)
{		
	idioma=1;
	mute=false; 
	slot=0;
	GraficosCPC=false;
	roms = romData + 0x4000;

	for (int i = 0; i < numSprites; i++){
		sprites[i] = 0;
	}

	for (int i = 0; i < numPersonajes; i++){
		personajes[i] = 0;
	}

	for (int i = 0; i < numPuertas; i++){
		puertas[i] = 0;
	}

	for (int i = 0; i < numObjetos; i++){
		objetos[i] = 0;
	}

	sys->initPaleta(romData+0x24000-1);	
	pergamino = new Pergamino();
	motor = new MotorGrafico(buffer, 8192);
	marcador = new Marcador();
	logica = new Logica(roms, buffer, 8192); 	
	infoJuego = new InfoJuego();
	
	pausaPorEstarEnMenus=false;
	modoInformacion = false;
	seleccionado = 0;
	
	currentState = Abadia::STATES::INTRO; 
	showingMenu = false;
	activeGame = false;
	
	#ifdef RG350	
	configReader = new ConfigReader("/usr/local/home/Abbey/config.txt");
	#else
#ifdef __EMSCRIPTEN__
	configReader = new ConfigReader("/save/config.txt");
#else
	configReader = new ConfigReader("config.txt");
#endif
	#endif

	selectedSlot = -1;

	checkConfigFile();
}

Juego::~Juego()
{
	for (int i = 0; i < numSprites; i++){
		delete sprites[i];
	}

	for (int i = 0; i < numPersonajes; i++){
		delete personajes[i];
	}

	for (int i = 0; i < numPuertas; i++){
		delete puertas[i];
	}

	for (int i = 0; i < numObjetos; i++){
		delete objetos[i];
	}

	delete infoJuego;
	delete logica;
	delete marcador;
	delete motor;
	delete pergamino;
    	delete configReader;
}

void Juego::ReiniciaPantalla(bool mostrarDiaYMomentoDia)
{
	limpiaAreaJuego(12);

	marcador->dibujaMarcador();

	motor->posXPantalla = motor->posYPantalla = -1;

	marcador->dibujaObjetos(personajes[0]->objetos, 0xff);

	if (mostrarDiaYMomentoDia) marcador->muestraDiaYMomentoDia();
	marcador->decrementaObsequium(0);
	marcador->limpiaAreaFrases();
}
/*
void Juego::pintaMenuCargar(int seleccionado,bool efecto)
{
	pintaMenuGrabar(seleccionado,efecto);
}

bool Juego::menuCargar2()
{
	pintaMenuCargar(seleccionado,true);
	int i = 0;	
	if (sys->pad.up)
	{
		i--;
		sys->pad.up = false;
	}		
	else if (sys->pad.down)
	{
		i++;
		sys->pad.down = false;
	}				
	seleccionado += i;
	if (seleccionado > 7){
		seleccionado = 0;
	}
	else if (seleccionado < 0){
		seleccionado = 7;
	}	
	pintaMenuCargar(seleccionado,true);
	
	if (BUTTON_YES)
	{
		BUTTON_YES = false;
		if (seleccionado != 7)
		{
			selectedSlot = seleccionado;

			if (activeGame)
			{
				changeState(Abadia::STATES::ASK_CONTINUE);
				ReiniciaPantalla();
				marcador->limpiaAreaMarcador();	
				return false;
			}

			laLogica->inicia();			
			cargar(seleccionado);			
			changeState(Abadia::STATES::PLAY);
			ReiniciaPantalla();
		}
		else
		{
			seleccionado = 4;
			changeState(Abadia::STATES::MENU);
		}		
	}
	return false;
}
*/
bool Juego::menuCargar()
{
    if (loadMenu.isEmpty()) {
        loadMenu.clear();
        for (int i = 0; i < 7; ++i)
            loadMenu.add([this, i]() { return saveFile[i]; }, [this, i]() {
                selectedSlot = i;
                if (activeGame) changeState(STATES::ASK_CONTINUE);
                else { logica->inicia(); cargar(i); changeState(STATES::PLAY); }
            });
        loadMenu.add([this]() { return textSave[idioma]; }, [this]() { changeState(STATES::MENU); });
    }
    return loadMenu.tick(*marcador);
}

bool Juego::cargar(int slot)
{	
	std::string path = "";

#ifdef __EMSCRIPTEN__
	path="/save/";
#endif

	#ifdef RG350
	path = "/usr/local/home/Abbey/";
	#endif

	#ifdef ANDROID
	if (SDL_AndroidGetExternalStorageState() != 0){
		path = SDL_AndroidGetExternalStoragePath();
		path += "/";
	}
	#endif
	
	#ifdef VITA
	path = VITA_SAVE_DIR;
	#endif

	std::ifstream in((path + savefile[slot]).c_str());
	in >> logica;
	if (in.fail())
	{
		elMarcador->imprimeFrase("                  ", 100, 164, 4, 0);
		elMarcador->imprimeFrase("ERROR: PRESS SPACE", 100, 164, 4, 0);
		elMarcador->imprimeFrase("                  ", 100, 164, 4, 0);
		logica->inicia();
		return true;
	}
	else {return true;}
}

string Juego::getDateAndTime()
{
	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
  	char buff[100];
  
  	snprintf(buff, sizeof(buff), "%d.%02d.%02d-%02d:%02d\n", 
	  tm.tm_mon + 1, tm.tm_mday, tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
	std::string buffAsStdStr = buff;
  	return buffAsStdStr;
}
/*
void Juego::askExitLogic()
{
	int i = 0;
	
	if (sys->pad.left)
	{
		i++;
		sys->pad.left = false;
	}		
	else if (sys->pad.right)
	{
		i--;
		sys->pad.right = false;
	}				
	seleccionado += i;
	if (seleccionado > 1){
		seleccionado = 0;
	}
	else if (seleccionado < 0){
		seleccionado = 1;
	}	

	if (BUTTON_YES)
	{
		BUTTON_YES = false;

		if (seleccionado == 0){				
			sys->exitGame();
		}
		else if (seleccionado==1)
		{
			changeState(Abadia::STATES::PLAY);
			ReiniciaPantalla();
			sys->setGamePalette(2);
			marcador->limpiaAreaMarcador();	
			ReiniciaPantalla();
			BUTTON_YES = false;

			sys->setNormalSpeed();
			activeGame = true;
		}
	}	
}

void Juego::askExit()
{
	int x = 0;	

	askExitLogic();

	limpiaAreaJuego(0); 
	
	x = 32;

	int delimiterPosition = continueQuestionText[idioma].find("\n");
	
	string line1 = continueQuestionText[idioma].substr(0, delimiterPosition);
	string line2 = continueQuestionText[idioma].substr(delimiterPosition+1,
		continueQuestionText[idioma].length());

	x = (320 - line1.length()*8)>>1;
	marcador->imprimeFrase(line1, x, 32, 4, 0);
	x = (320 - line2.length()*8)>>1;
	marcador->imprimeFrase(line2, x, 43, 4, 0);

	const int x1 = 140;
	marcador->imprimeFrase(yesText[idioma], x1, 64, 4, 0);
	const int x2 = 170;
	marcador->imprimeFrase(noText[idioma], x2, 64, 4, 0);

	if (seleccionado == 0){
		marcador->imprimeFrase(yesText[idioma], x1, 64, 0, 4);
	}
	else{		
		marcador->imprimeFrase(noText[idioma], x2, 64, 0, 4);
	}
}
*/

void Juego::askExit()
{
    if (askExitMenu.isEmpty()) {
        askExitMenu.clear();
        askExitMenu.clear(); askExitMenu.setOrientation(MenuOrientation::HORIZONTAL);
//        askExitMenu.setPrompt("¿Deseas salir?\nPerderás el progreso actual.");
//        askExitMenu.setPrompt(continueQuestionText[idioma]);
	askExitMenu.setPrompt([this]() { return continueQuestionText[idioma]; });
        askExitMenu.add([this]() { return yesText[idioma]; }, [this]() { sys->exitGame(); });
        askExitMenu.add([this]() { return noText[idioma]; }, [this]() {
            changeState(STATES::PLAY); ReiniciaPantalla(); activeGame = true; sys->setNormalSpeed();
        });
    }
    askExitMenu.tick(*marcador);
}
/*
void Juego::askForNewGameLogic()
{	
	int i = 0;
	
	if (sys->pad.left)
	{
		i++;
		sys->pad.left = false;
	}		
	else if (sys->pad.right)
	{
		i--;
		sys->pad.right = false;
	}				
	seleccionado += i;
	if (seleccionado > 1){
		seleccionado = 0;
	}
	else if (seleccionado < 0){
		seleccionado = 1;
	}	

	if (BUTTON_YES)
	{
		BUTTON_YES = false;

		if (seleccionado == 0){				
			logica->inicia();		
		}

		changeState(Abadia::STATES::PLAY);
		ReiniciaPantalla();
		sys->setGamePalette(2);
		marcador->limpiaAreaMarcador();	
		ReiniciaPantalla();
		BUTTON_YES = false;

		sys->setNormalSpeed();
		activeGame = true;
	}
}

void Juego::askForNewGame()
{
	int x = 0;	

	askForNewGameLogic();

	limpiaAreaJuego(0); 
	
	x = 32;

	int delimiterPosition = newGameQuestionText[idioma].find("\n");
	
	string line1 = newGameQuestionText[idioma].substr(0, delimiterPosition);
	string line2 = newGameQuestionText[idioma].substr(delimiterPosition+1,
		newGameQuestionText[idioma].length());

	x = (320 - line1.length()*8)>>1;
	marcador->imprimeFrase(line1, x, 32, 4, 0);
	x = (320 - line2.length()*8)>>1;
	marcador->imprimeFrase(line2, x, 43, 4, 0);

	const int x1 = 140;
	marcador->imprimeFrase(yesText[idioma], x1, 64, 4, 0);
	const int x2 = 170;
	marcador->imprimeFrase(noText[idioma], x2, 64, 4, 0);

	if (seleccionado == 0){
		marcador->imprimeFrase(yesText[idioma], x1, 64, 0, 4);
	}
	else{		
		marcador->imprimeFrase(noText[idioma], x2, 64, 0, 4);
	}
}
*/
void Juego::askForNewGame()
{
    if (askNewMenu.isEmpty()) {
        askNewMenu.clear(); askNewMenu.setOrientation(MenuOrientation::HORIZONTAL);
        //askNewMenu.setPrompt(newGameQuestionText[idioma]);
	askNewMenu.setPrompt([this]() { return newGameQuestionText[idioma]; });
        askNewMenu.add([this]() { return yesText[idioma]; }, [this]() { logica->inicia(); changeState(STATES::PLAY); ReiniciaPantalla(); });
        askNewMenu.add([this]() { return noText[idioma]; }, [this]() { changeState(STATES::PLAY); ReiniciaPantalla(); });
    }
    askNewMenu.tick(*marcador);
}
/*
void Juego::askToContinueLogic()
{	
	int i = 0;
	
	if (sys->pad.left)
	{
		i++;
		sys->pad.left = false;
	}		
	else if (sys->pad.right)
	{
		i--;
		sys->pad.right = false;
	}				
	seleccionado += i;
	if (seleccionado > 1){
		seleccionado = 0;
	}
	else if (seleccionado < 0){
		seleccionado = 1;
	}	

	if (BUTTON_YES)
	{
		BUTTON_YES = false;
		if (seleccionado == 1) //NO
		{	
			changeState(Abadia::STATES::PLAY);
			ReiniciaPantalla();
			sys->setGamePalette(2);
			marcador->limpiaAreaMarcador();	
			ReiniciaPantalla();
			BUTTON_YES = false;

			sys->setNormalSpeed();
			activeGame = true;		
		}
		else //YES
		{	
			laLogica->inicia();
			cargar(selectedSlot);						
			changeState(Abadia::STATES::PLAY);
			ReiniciaPantalla();
		}
	}
}

void Juego::askToContinue()
{
	int x = 0;	

	askToContinueLogic();

	limpiaAreaJuego(0); 
	
	x = 32;

	int delimiterPosition = continueQuestionText[idioma].find("\n");
	
	string line1 = continueQuestionText[idioma].substr(0, delimiterPosition);
	string line2 = continueQuestionText[idioma].substr(delimiterPosition+1,
		continueQuestionText[idioma].length());

	x = (320 - line1.length()*8)>>1;
	marcador->imprimeFrase(line1, x, 32, 4, 0);
	x = (320 - line2.length()*8)>>1;
	marcador->imprimeFrase(line2, x, 43, 4, 0);

	const int x1 = 140;
	marcador->imprimeFrase(yesText[idioma], x1, 64, 4, 0);
	const int x2 = 170;
	marcador->imprimeFrase(noText[idioma], x2, 64, 4, 0);

	if (seleccionado == 0){
		marcador->imprimeFrase(yesText[idioma], x1, 64, 0, 4);
	}
	else{		
		marcador->imprimeFrase(noText[idioma], x2, 64, 0, 4);
	}
}
*/
void Juego::askToContinue()
{
    if (askContMenu.isEmpty()) {
        askContMenu.clear(); askContMenu.setOrientation(MenuOrientation::HORIZONTAL);
        //askContMenu.setPrompt(continueQuestionText[idioma]);
	askContMenu.setPrompt([this]() { return continueQuestionText[idioma]; });
        askContMenu.add([this]() { return yesText[idioma]; }, [this]() {
            logica->inicia(); cargar(selectedSlot); changeState(STATES::PLAY); ReiniciaPantalla();
        });
        askContMenu.add([this]() { return noText[idioma]; }, [this]() { changeState(STATES::PLAY); ReiniciaPantalla(); });
    }
    askContMenu.tick(*marcador);
}

void Juego::save(int slot)
{
	string d = getDateAndTime();
	string token = "SAVEX";
	token[4] = '0' + slot;		
	configReader->setValue(token, d);
	saveConfigFile();

	std::string path = "";
#ifdef __EMSCRIPTEN__
	path="/save/";
#endif

	#ifdef RG350
	path = "/usr/local/home/Abbey/";
	#endif

	#ifdef ANDROID
	if (SDL_AndroidGetExternalStorageState() != 0){
		path = SDL_AndroidGetExternalStoragePath();
		path += "/";
	}
	#endif
	
	#ifdef VITA
	path = VITA_SAVE_DIR;
	#endif

	std::ofstream out((path + savefile[slot]).c_str(),
			std::ofstream::out|std::ofstream::trunc);
	
	out << logica; 

	if (out.fail())
	{
		elMarcador->imprimeFrase("                  ", 100, 164, 4, 0);
		elMarcador->imprimeFrase("ERROR: PRESS SPACE", 100, 164, 4, 0);		
	}
}
/*
void Juego::pintaMenuGrabar(int seleccionado,bool efecto)
{	
	limpiaAreaJuego(0); 
	marcador->limpiaAreaMarcador();	

	int x = 0;
	const int y = 32;
	sys->fillMode1Rect(8, 0, 88, 160, 0);
	for (int i=0;i<7;i++)
	{
		if (i == seleccionado){
			x = (320 - saveFile[i].length()*8)>>1;
			marcador->imprimeFrase(saveFile[i], x, y+(i*16),0, 4);
		}
		else{
			x = (320 - saveFile[i].length()*8)>>1;
			marcador->imprimeFrase(saveFile[i], x, y+(i*16),4, 0);
		}		
	}
	if (seleccionado == 7){
		x = (320 - textSave[idioma].length()*8)>>1;
		marcador->imprimeFrase(textSave[idioma], x, y+(7*16),0, 4);
	}
	else{
		x = (320 - textSave[idioma].length()*8)>>1;
		marcador->imprimeFrase(textSave[idioma], x, y+(7*16),4, 0);
	}
}

bool Juego::menuGrabar2()
{
	pintaMenuGrabar(seleccionado,true);
	int i = 0;	
	if (sys->pad.up)
	{
		i--;
		sys->pad.up = false;
	}		
	else if (sys->pad.down)
	{
		i++;
		sys->pad.down = false;
	}				
	seleccionado += i;
	if (seleccionado > 7){
		seleccionado = 0;
	}
	else if (seleccionado < 0){
		seleccionado = 7;
	}	
	pintaMenuCargar(seleccionado,true);	
	if (BUTTON_YES)
	{
		BUTTON_YES = false;
		if (seleccionado != 7)
		{			
			save(seleccionado);						
			changeState(Abadia::STATES::PLAY);
			ReiniciaPantalla();
			return true;
		}
		else
		{
			seleccionado = 4;
			changeState(Abadia::STATES::MENU);
		}
	}
	return false;	
}
*/
bool Juego::menuGrabar()
{
    if (saveMenu.isEmpty()) {
        saveMenu.clear();
        for (int i = 0; i < 7; ++i)
            saveMenu.add([this, i]() { return saveFile[i]; }, [this, i]() {
                save(i); changeState(STATES::PLAY); ReiniciaPantalla();
            });
        saveMenu.add([this]() { return textSave[idioma]; }, [this]() { changeState(STATES::MENU); });
    }
    return saveMenu.tick(*marcador);
}

/*
void Juego::pintaMenuIdioma(int seleccionado,bool efecto)
{
	limpiaAreaJuego(0); 

	int x = 0;
	
	for (int i=0;i<8;i++){
		x = (320 - textLanguage[i].length()*8)>>1;
		marcador->imprimeFrase(textLanguage[i], x, 32+(i*16),4, 0);
	}

	x = (320 - textLanguage[seleccionado].length()*8)>>1;
	marcador->imprimeFrase(textLanguage[seleccionado], x, 
		32+(seleccionado*16), 0, 4);
}

bool Juego::menuIdioma()
{
	limpiaAreaJuego(0);

	pintaMenuIdioma(seleccionado,true);
				
	if (sys->pad.down) {
		seleccionado++;		
		sys->pad.down = false;
	}
	else if (sys->pad.up) {
		seleccionado--;						
		sys->pad.up = false;
	}
	if (seleccionado==8) seleccionado=0;
	else if (seleccionado==-1) seleccionado=7;

	pintaMenuIdioma(seleccionado);

	if (BUTTON_YES)
	{
		BUTTON_YES = false;
		idioma=seleccionado;

		string d = getDateAndTime();
		string token = "LANGUAGE";
		token[4] = '0' + idioma;		
		configReader->setValue(token, d);
		saveConfigFile();

		seleccionado = 4;
		changeState(Abadia::STATES::MENU);
		return true;
	}	
	
	return false;
}
*/
bool Juego::menuIdioma()
{
    if (langMenu.isEmpty()) {
        langMenu.clear();
        for (int i = 0; i < 8; ++i) {
            langMenu.add([this, i]() { return textLanguage[i]; }, [this, i]() {
                idioma = i;
                std::string d = getDateAndTime();
                std::string token = "LANGUAGE"; token[4] = '0' + idioma;
                configReader->setValue(token, d); saveConfigFile();
                seleccionado = 4; changeState(STATES::MENU);
            });
        }
    }
    return langMenu.tick(*marcador);
}


/*
void Juego::pintaMenuPrincipal(int seleccionado,bool efecto)
{
	limpiaAreaJuego(0); 

	int x = 0;
	const int y = 32;

	for (int i=0;i<6;i++)
	{						
		if ((i==4 || i==2) && !activeGame){
			x = (320 - principalMenuText[idioma][i].length()*8)>>1;
			marcador->imprimeFrase(principalMenuText[idioma][i], x, y+(i*16),5, 0);
		}
		else{
			x = (320 - principalMenuText[idioma][i].length()*8)>>1;
			marcador->imprimeFrase(principalMenuText[idioma][i], x, y+(i*16),4, 0);
		}
	}
	
	x = (320 - principalMenuText[idioma][seleccionado].length()*8)>>1;
	marcador->imprimeFrase(principalMenuText[idioma][seleccionado], x, 
		y+(seleccionado*16), 0, 4);	
}

bool Juego::menu()
{	
	if (BUTTON_YES)
	{
		BUTTON_YES = false;
		switch(seleccionado)
		{
			case 0: //New game					
				if (!activeGame)
				{									
					changeState(Abadia::STATES::SCROLL);
					ReiniciaPantalla();
					sys->setGamePalette(2);
					marcador->limpiaAreaMarcador();	
					ReiniciaPantalla();
					BUTTON_YES = false;

					sys->minimumFrameTime = SCROLL_FRAME_TIME;
					sys->playSound(Abadia::SONIDOS::Inicio);
					activeGame = true;						
				}
				else
				{
					seleccionado = 1;
					changeState(Abadia::STATES::ASK_NEW_GAME);
					ReiniciaPantalla();
					sys->setGamePalette(2);
					marcador->limpiaAreaMarcador();					
					BUTTON_YES = false;

					activeGame = true;	
					return true;
				}
				return true;		
				break;		
			case 1: //Load				
				checkForSaveFiles();				
				changeState(Abadia::STATES::LOAD);
				ReiniciaPantalla();
				sys->setGamePalette(2);
				marcador->limpiaAreaMarcador();					
				BUTTON_YES = false;

				activeGame = true;
				
				return true;
				break;
			case 2: //save
				if (activeGame)
				{
					checkForSaveFiles();
					changeState(Abadia::STATES::SAVE);
					ReiniciaPantalla();				
					sys->setGamePalette(2);
					marcador->limpiaAreaMarcador();						
					BUTTON_YES = false;

					activeGame = true;	
				}
				return true;
				break;
			case 3: //Language
				seleccionado = idioma;
				changeState(Abadia::STATES::LANGUAGE);
				ReiniciaPantalla();
				sys->setGamePalette(2);
				marcador->limpiaAreaMarcador();					
				BUTTON_YES = false;

				return true;
				break;					
			case 4: //Continue
				if (activeGame)
				{
					changeState(Abadia::STATES::PLAY);
					ReiniciaPantalla();
					activeGame = true;	
					return true;
				}
				break;
			case 5: //EXIT
				seleccionado = 1;
				changeState(Abadia::STATES::ASK_EXIT);
				ReiniciaPantalla();				
				sys->setGamePalette(2);
				marcador->limpiaAreaMarcador();						
				BUTTON_YES = false;

				break;
		}
	}

	pintaMenuPrincipal(seleccionado,true);		
	
	if (sys->pad.up)
	{
		seleccionado--;
		sys->pad.up = false;
	}		
	else if (sys->pad.down)
	{
		seleccionado++;
		sys->pad.down = false;
	}					

	if (seleccionado > 5){
		seleccionado = 0;
	}	
	else if (seleccionado < 0){
		seleccionado = 5;
	}	

	return false;
}
*/
// En Juego.h, añade como miembro privado:
// SimpleMenu mainMenu;
// bool menuInitialized = false;

bool Juego::menu()
{
    //if (!mainMenuReady) {
    if (mainMenu.isEmpty()) {
        mainMenu.clear();
        
        // Item 0: Nueva Partida
        mainMenu.add(
            [this]() { return principalMenuText[idioma][0]; },
            [this]() {
                if (!activeGame) {
                    changeState(Abadia::STATES::SCROLL);
                    sys->setGamePalette(2);
                    marcador->limpiaAreaMarcador();
                    ReiniciaPantalla();
                    sys->minimumFrameTime = SCROLL_FRAME_TIME;
                    sys->playSound(Abadia::SONIDOS::Inicio);
                    activeGame = true;
                } else {
                    changeState(Abadia::STATES::ASK_NEW_GAME);
                }
            }
        );

        // Item 1: Cargar
        mainMenu.add([this]() { return principalMenuText[idioma][1]; }, [this]() {
            checkForSaveFiles();
            changeState(Abadia::STATES::LOAD);
        });

        // Item 2: Guardar (deshabilitado si no hay partida)
        mainMenu.add([this]() { return principalMenuText[idioma][2]; }, [this]() {
            checkForSaveFiles();
            changeState(Abadia::STATES::SAVE);
        }, [this]() { return activeGame; });

        // Item 3: Idioma
        mainMenu.add([this]() { return principalMenuText[idioma][3]; }, [this]() {
            seleccionado = idioma;
            changeState(Abadia::STATES::LANGUAGE);
        });

        // Item 4: Continuar (deshabilitado si no hay partida)
        mainMenu.add([this]() { return principalMenuText[idioma][4]; }, [this]() {
            changeState(Abadia::STATES::PLAY);
            activeGame = true;
        }, [this]() { return activeGame; });

        // Item 5: Salir
        mainMenu.add([this]() { return principalMenuText[idioma][5]; }, [this]() {
            changeState(Abadia::STATES::ASK_EXIT);
        });
    }

    return mainMenu.tick(*marcador);
}

/////////////////////////////////////////////////////////////////////////////
// método principal del juego
/////////////////////////////////////////////////////////////////////////////
void Juego::preRun()
{	
	marcador->limpiaAreaMarcador();

	creaEntidadesJuego();
	generaGraficosFlipeados();
	motor->personaje = personajes[0];
	infoJuego->inicia();
	logica->despHabitacionEspejo();
	logica->inicia();

	marcador->limpiaAreaMarcador();
	logica->inicia();
	ReiniciaPantalla();
}
/*
void Juego::changeState(Abadia::STATES newState)
{
	ReiniciaPantalla();
	marcador->limpiaAreaMarcador();

	switch (newState)
	{
		case STATES::PLAY:
			pausaPorEstarEnMenus=false;
			break;
		case STATES::INTRO:			
		case STATES::LANGUAGE:			
		case STATES::MENU:
		case STATES::LOAD:
		case STATES::SAVE:
		case STATES::SCROLL:
		case STATES::ASK_NEW_GAME:
		case STATES::ASK_CONTINUE:
		case STATES::ASK_EXIT:
		case STATES::ENDING:
			pausaPorEstarEnMenus=true;
			break;
	}

	currentState = newState;
}
*/

void Juego::changeState(Abadia::STATES newState)
{
	ReiniciaPantalla();
	marcador->limpiaAreaMarcador();

	// limpieza según el estado del que salimos
	switch (currentState)
	{
		case STATES::SCROLL:
			// el pergamino de introducción tiene su propio sonido;
			// lo paramos siempre al salir, independientemente del destino
			sys->stopSound(Abadia::SONIDOS::Inicio);
			break;
		case STATES::ENDING:
			sys->stopSound(Abadia::SONIDOS::Final);
			break;
		default:
			break;
	}

	// ajuste de pausa y sonidos según el estado al que entramos
	switch (newState)
	{
		case STATES::PLAY:
			pausaPorEstarEnMenus = false;
			// solo reanudamos si veníamos de un estado de menú,
			// no si veníamos de SCROLL o ENDING que tienen sus propios sonidos
			if (currentState != STATES::SCROLL && currentState != STATES::ENDING)
				sys->resumeSounds();
			ReiniciaPantalla();
			break;

		case STATES::INTRO:
		case STATES::LANGUAGE:
		case STATES::MENU:
		case STATES::LOAD:
		case STATES::SAVE:
		case STATES::ASK_NEW_GAME:
		case STATES::ASK_CONTINUE:
		case STATES::ASK_EXIT:
			pausaPorEstarEnMenus = true;
			// congelamos los sonidos de juego sin detenerlos
			sys->pauseSounds();
			sys->setGamePalette(2);
			limpiaAreaJuego(0);
			break;

		case STATES::SCROLL:
		case STATES::ENDING:
			// estados con su propia banda sonora; no pausamos ni reanudamos
			// los sonidos de juego porque en estos estados no hay partida activa
			pausaPorEstarEnMenus = true;
			break;
	}

	currentState = newState;
}

void Juego::run()
{
	elBuscadorDeRutas->contadorAnimGuillermo = laLogica->guillermo->contadorAnimacion;
	
	logica->compruebaAbreEspejo();
	logica->actualizaVariablesDeTiempo();

	if (muestraPantallaFinInvestigacion()) return;		
	
	logica->compruebaLecturaLibro();	
	marcador->realizaScrollMomentoDia();		
	logica->ejecutaAccionesMomentoDia();
	logica->compruebaBonusYCambiosDeCamara();
	motor->compruebaCambioPantalla();
	logica->compruebaCogerDejarObjetos();
	logica->compruebaAbrirCerrarPuertas();
			
	for (int i = 0; i < numPersonajes; i++){
		personajes[i]->run();
	}
	
	logica->buscRutas->generadoCamino = false;	
	
	actualizaLuz();

	laLogica->realizaReflejoEspejo();
	
	if (cambioModoInformacion && modoInformacion)
	{
		cambioModoInformacion=false;
	}

	if (cambioModoInformacion && !modoInformacion)
	{
		limpiaAreaJuego(12);
		motor->compruebaCambioPantalla(true);	
		cambioModoInformacion=false;			
	}

	{		
		motor->dibujaPantalla();
		motor->dibujaSprites();				
	}
	if (modoInformacion){
		infoJuego->muestraInfo();
	}

	if (laLogica->guillermo->contadorAnimacion==1){			
		sys->playSound(Abadia::SONIDOS::Pasos);
	}
}

void Juego::limpiaAreaJuego(int color)
{
	sys->fillMode1Rect(0, 0, 32, 160, 0);
	sys->fillMode1Rect(32, 0, 256, 160, color);
	sys->fillMode1Rect(32 + 256, 0, 32, 160, 0);	
}

void Juego::generaGraficosFlipeados()
{
	generaGraficosFlipeadosVGA();

	UINT8 tablaFlipX[256];

	for (int i = 0; i < 256; i++)
	{
		int pixel0 = unpackPixelMode1(i, 0);
		int pixel1 = unpackPixelMode1(i, 1);
		int pixel2 = unpackPixelMode1(i, 2);
		int pixel3 = unpackPixelMode1(i, 3);

		int data = 0;

		data = packPixelMode1(data, 0, pixel3);
		data = packPixelMode1(data, 1, pixel2);
		data = packPixelMode1(data, 2, pixel1);
		data = packPixelMode1(data, 3, pixel0);

		tablaFlipX[i] = data;
	}

	flipeaGraficos(tablaFlipX, &roms[0x0a300], &roms[0x16300], 5, 0x366);
	flipeaGraficos(tablaFlipX, &roms[0x0a666], &roms[0x16666], 4, 0x084);
	flipeaGraficos(tablaFlipX, &roms[0x0a6ea], &roms[0x166ea], 5, 0x1db);
	flipeaGraficos(tablaFlipX, &roms[0x0a8c5], &roms[0x168c5], 4, 0x168);
	flipeaGraficos(tablaFlipX, &roms[0x0ab59], &roms[0x16b59], 5, 0x2d5);
	flipeaGraficos(tablaFlipX, &roms[0x0b103], &roms[0x17103], 5, 0x2bc);
	flipeaGraficos(tablaFlipX, &roms[0x0aa49], &roms[0x16a49], 6, 0x0f0);
}

void Juego::generaGraficosFlipeadosVGA()
{
	UINT8 *romsVGA = &roms[0x24000-1-0x4000];
	UINT8 *romsVGAFlip = &roms[0x24000 + 174065 -1 - 0x4000];
	int dest = 0;
	int size = 57240-53760;

	flipeaGraficosVGA(&romsVGA[53760], &romsVGAFlip[dest], 5*4, size);
	dest += size;
	size = 57768-57240;
	flipeaGraficosVGA(&romsVGA[57240], &romsVGAFlip[dest], 4*4, size);
	dest += size;

	size = 59668 - 57768;
	flipeaGraficosVGA(&romsVGA[57768], &romsVGAFlip[dest], 5*4, size);
	dest += size;
	size = 61108 - 59668;
	flipeaGraficosVGA(&romsVGA[59668], &romsVGAFlip[dest], 4*4, size);

	dest += size;
	size = 64008 - 61108;
	flipeaGraficosVGA(&romsVGA[61108], &romsVGAFlip[dest], 5*4, size);

	dest += size;
	dest+=2900;
	size = 69708 - 66908;
	flipeaGraficosVGA(&romsVGA[66908], &romsVGAFlip[dest], 5*4, size);

	dest += size;
	size = 24*40;
	flipeaGraficosVGA(&romsVGA[69708], &romsVGAFlip[dest], 6*4, size);
}

void Juego::flipeaGraficos(UINT8 *tablaFlip, UINT8 *src, UINT8 *dest, int ancho, int bytes)
{
	memcpy(dest, src, bytes);

	int numLineas = bytes/ancho;
	int numIntercambios = (ancho + 1)/2;

	for (int j = 0; j < numLineas; j++)
	{
		UINT8 *ptr1 = dest;
		UINT8 *ptr2 = ptr1 + ancho - 1;

		for (int i = 0; i < numIntercambios; i++)
		{
			UINT8 aux = *ptr1;
			*ptr1 = tablaFlip[*ptr2];
			*ptr2 = tablaFlip[aux];

			ptr1++;
			ptr2--;
		}

		dest = dest + ancho;
	}
}

void Juego::flipeaGraficosVGA(UINT8 *src, UINT8 *dest, int ancho, int bytes)
{
	memcpy(dest, src, bytes);

	int numLineas = bytes/ancho;
	int numIntercambios = (ancho + 1)/2;

	for (int j = 0; j < numLineas; j++)
	{
		UINT8 *ptr1 = dest;
		UINT8 *ptr2 = ptr1 + ancho - 1;

		for (int i = 0; i < numIntercambios; i++)
		{
			UINT8 aux = *ptr1;
			*ptr1 = *ptr2;
			*ptr2 = aux;

			ptr1++;
			ptr2--;
		}

		dest = dest + ancho;
	}
}

void Juego::actualizaLuz()
{
	sprites[spriteLuz]->esVisible = false;

	if (motor->pantallaIluminada) return;

	if (!(personajes[1]->sprite->esVisible))
	{
		for (int i = 0; i < numSprites; i++)
		{
			if (sprites[i]->esVisible){
				sprites[i]->haCambiado = false;
			}
		}
		return;
	}

	SpriteLuz *sprLuz = (SpriteLuz *) sprites[spriteLuz];
	sprLuz->ajustaAPersonaje(personajes[1]);
}

void Juego::cambioCPC_VGA()
{
	if (GraficosCPC)
	{
		memcpy(	&roms[0x24000-1-0x4000],
				&roms[0x24000-1-0x4000+(174065+21600)],
				174065);
		GraficosCPC=false;
	}
	else
	{
		memcpy(	&roms[0x24000-1-0x4000],
				&roms[0x24000-1-0x4000+(174065+21600)*2],
				174065);
		GraficosCPC=true;
	}

	generaGraficosFlipeados();
	ReiniciaPantalla();		
}

/////////////////////////////////////////////////////////////////////////////
// métodos para mostrar distintas pantallas
/////////////////////////////////////////////////////////////////////////////

void Juego::muestraPresentacion()
{
	sys->setIntroPalette();
	UINT8 *romsVGA = &roms[0x24000-1-0x4000];
	UINT8 *screen=romsVGA+0x1ADF0;
	for (int j = 0; j < 200; j++){
		for (int i = 0; i < 320; i++){
			sys->setPixel(i,j,*screen++);
		}
	}

	if (BUTTON_YES)
	{
		currentState = Abadia::STATES::MENU;
		ReiniciaPantalla();
		marcador->limpiaAreaMarcador();
	}
}

void Juego::muestraIntroduccion()
{
	pergamino->muestraTexto(Pergamino::pergaminoInicio[idioma]);
	
	if (pergamino->finished)
	{
		sys->setGamePalette(0);
		currentState = Abadia::STATES::PLAY;
	
		sys->stopSound(Abadia::SONIDOS::Inicio);
		ReiniciaPantalla();
		sys->setGamePalette(2);
		marcador->limpiaAreaMarcador();

		ReiniciaPantalla();
		BUTTON_YES = false;
		
		sys->setNormalSpeed();
	}
}

void Juego::muestraFinal()
{
	sys->playSound(Abadia::SONIDOS::Final,true);
	pergamino->muestraTexto(Pergamino::pergaminoFinal[idioma]);
}

bool Juego::muestraPantallaFinInvestigacion()
{
	std::string porcentaje[8] = {
	"XX POR CIENTO DE",
	"XX  PER  CENT",
	"XX POR CENTO DA",
	"XX PER CENT DE",
	"XX POR CENTO DA",
	"XX PER CENTO",
	"XX  PER  CENT",
	"XX POR CENTO DA"
	};

	if (!logica->haFracasado) {return false;}

	laLogica->numPersonajeCamara = 0x80;

	if (elGestorFrases->mostrandoFrase) {return false;}

	limpiaAreaJuego(0);

	int porc = logica->calculaPorcentajeMision();

	porcentaje[idioma][0] = ((porc/10) % 10) + 0x30;
	porcentaje[idioma][1] = (porc % 10) + 0x30;
	
	int x = 0;
	x = (320 - frase1[idioma].length()*8)>>1;
	marcador->imprimeFrase(frase1[idioma], x, 32, 4, 0);
	x = (320 - porcentaje[idioma].length()*8)>>1;
	marcador->imprimeFrase(porcentaje[idioma], x, 48, 4, 0);
	x = (320 - frase3[idioma].length()*8)>>1;
	marcador->imprimeFrase(frase3[idioma], x, 64, 4, 0);
	x = (320 - frase4[idioma].length()*8)>>1;
	marcador->imprimeFrase(frase4[idioma], x, 128, 4, 0);

	if (sys->pad.button1 ||sys->pad.button2 ||sys->pad.button3 ||sys->pad.button4)
	{
		sys->setIntroPalette();
		UINT8 *romsVGA = &roms[0x24000-1-0x4000];
		UINT8 *screen=romsVGA+0x1ADF0;
		for (int j = 0; j < 200; j++){
			for (int i = 0; i < 320; i++){
				sys->setPixel(i,j,*screen++);
			}
		}

		currentState = Abadia::STATES::INTRO;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// creación de las entidades del juego
/////////////////////////////////////////////////////////////////////////////

void Juego::creaEntidadesJuego()
{
	sprites[0] = new Sprite();
	sprites[1] = new Sprite();

	for (int i = 2; i < 8; i++){
		sprites[i] = new SpriteMonje();
	}

	for (int i = primerSpritePuertas; i < primerSpritePuertas + numPuertas; i++)
	{
		sprites[i] = new Sprite();
		sprites[i]->ancho = sprites[i]->oldAncho = 0x06;
		sprites[i]->alto = sprites[i]->oldAlto = 0x28;
	}

	int despObjetos[8] = { 
		11200,
		34496,
		34304,
		34880,
		34688,
		34688,
		34688,
		11008
	};

	for (int i = primerSpriteObjetos; i < primerSpriteObjetos + numObjetos; i++)
	{
		sprites[i] = new Sprite();
		sprites[i]->ancho = sprites[i]->oldAncho = 0x04;
		sprites[i]->alto = sprites[i]->oldAlto = 0x0c;
		sprites[i]->despGfx = despObjetos[i - primerSpriteObjetos];
	}

	sprites[spritesReflejos] = new Sprite();
	sprites[spritesReflejos + 1] = new Sprite();

	sprites[spriteLuz] = new SpriteLuz();

	personajes[0] = new Guillermo(sprites[0]);
	personajes[1] = new Adso(sprites[1]);
	personajes[2] = new Malaquias((SpriteMonje *)sprites[2]);
	personajes[3] = new Abad((SpriteMonje *)sprites[3]);
	personajes[4] = new Berengario((SpriteMonje *)sprites[4]);
	personajes[5] = new Severino((SpriteMonje *)sprites[5]);
	personajes[6] = new Jorge((SpriteMonje *)sprites[6]);
	personajes[7] = new Bernardo((SpriteMonje *)sprites[7]);

	for (int i = 0; i < 8; i++){
		personajes[i]->despX = -2;
		personajes[i]->despY = -34;
	}
	personajes[1]->despY = -32;
	
	for (int i = 0; i < numPuertas; i++){
		puertas[i] = new Puerta(sprites[primerSpritePuertas + i]);
	}

	for (int i = 0; i < numObjetos; i++){
		objetos[i] = new Objeto(sprites[primerSpriteObjetos + i]);
	}
}

void Juego::checkForSaveFiles()
{
	checkConfigFile();
	for (int i=0;i<7;i++)
	{
		string token = "SAVEX";
		token[4] = '0' + i;		
		saveFile[i] = configReader->getValue(token); 
	}
}

void Juego::checkConfigFile()
{
	if (!readConfigFile())
	{
		if (saveConfigFile()){
		}
	}
}

bool Juego::readConfigFile()
{
	bool r = false;

	if (!configReader->isEmpty()){
		delete configReader;
	}

	std::string path = "";

	#ifdef RG350
	path = "/usr/local/home/Abbey/";
	#endif

#ifdef __EMSCRIPTEN__
	path="/save/";
#endif

	#ifdef ANDROID
	if (SDL_AndroidGetExternalStorageState() != 0){
		path = SDL_AndroidGetExternalStoragePath();
		path += "/";
	}
	#endif
	
	#ifdef VITA
	path = VITA_SAVE_DIR;
	#endif

	configReader = new ConfigReader((path + "config.txt").c_str());

	if (configReader->parse())
	{
		string s = configReader->getValue("LANGUAGE");
		idioma = atoi(s.c_str());
		r = true;
	}

	return r;
}

bool Juego::saveConfigFile()
{
	bool r = false;
	ofstream f;

	std::string path = "";

	#ifdef RG350
	path = "/usr/local/home/Abbey/";
	#endif
#ifdef __EMSCRIPTEN__
	path="/save/";
#endif

	#ifdef ANDROID
	if (SDL_AndroidGetExternalStorageState() != 0){
		path = SDL_AndroidGetExternalStoragePath();
		path += "/";	
	}
	#endif
	
	#ifdef VITA
	path = VITA_SAVE_DIR;
	#endif
	
	f.open((path + "config.txt").c_str());

	f << "LANGUAGE="<< idioma <<"\n";

	for (int i=0;i<7;i++)
	{
		string token = "SAVEX";
		token[4] = '0' + i;

		if (!configReader->isEmpty()){			
			f << "SAVE" << i << "=" << configReader->getValue(token) <<"\n";
		}
		else{
			f << "SAVE" << i << "=" <<"--" <<"\n";
		}
	}

	f.close();
	return r;
}
