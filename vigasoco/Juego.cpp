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

using namespace Abadia;

/////////////////////////////////////////////////////////////////////////////
// inicialización y limpieza
/////////////////////////////////////////////////////////////////////////////
Juego::Juego(UINT8 *romData)
{		
	//sys->idioma = 1; // se leen desde el config
	//sys->mute = false; // se leen desde el config
	slot = 0;
	// sys->GraficosCPC = false;  // se leen desde el config
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

	pausaPorEstarEnMenus = false;
	modoInformacion = false;
	seleccionado = 0;

	currentState = Abadia::STATES::INTRO; 
	showingMenu = false;
	activeGame = false;
	selectedSlot = -1;

	// Leer configuración. Si hay preferencia de CPC la aplicamos ahora
	// (antes de que creaEntidadesJuego/generaGraficosFlipeados usen los datos).
	SDL_Log("mute debería ser falso 0 y es %d\n", (int)(bool)sys->mute);
	SDL_Log("mute deberia ser el del fichero %d\n", (int)(bool)sys->mute);
	
	// aplicaGraficos copia los datos correctos al buffer activo según GraficosCPC.
	// Los gráficos flipeados se generan después en preRun().
	aplicaGraficos(sys->GraficosCPC);
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
}

void Juego::ReiniciaPantalla(bool mostrarDiaYMomentoDia)
{
	limpiaAreaJuego(0);
	marcador->limpiaAreaMarcador();
	marcador->dibujaMarcador();

	motor->posXPantalla = motor->posYPantalla = -1;

	marcador->dibujaObjetos(personajes[0]->objetos, 0xff);

	if (mostrarDiaYMomentoDia) marcador->muestraDiaYMomentoDia();
	marcador->decrementaObsequium(0);
	marcador->limpiaAreaFrases();
}

/////////////////////////////////////////////////////////////////////////////
// helpers gráficos
/////////////////////////////////////////////////////////////////////////////
// Copia al buffer activo los datos VGA (slot índice 1 del bloque de roms)
// o los datos CPC (slot índice 2). Actualiza GraficosCPC.
// NO regenera flipeados ni toca paleta: el llamador decide cuándo hacerlo.
void Juego::aplicaGraficos(bool usarCPC)
{
	const int vgaSize   = 174065;
	const int flipExtra = 21600;
	// El layout en memoria (establecido en Abbey::filesLoaded) es:
	//   base                          -> VGA original (slot 0, usado en juego)
	//   base + vgaSize + flipExtra    -> copia VGA   (slot 1, fuente inmutable)
	//   base + (vgaSize+flipExtra)*2  -> datos CPC   (slot 2, fuente inmutable)
	UINT8 *base    = &roms[0x24000-1-0x4000];
	UINT8 *srcVGA  = base + (vgaSize + flipExtra);
	UINT8 *srcCPC  = base + (vgaSize + flipExtra) * 2;

	memcpy(base, usarCPC ? srcCPC : srcVGA, vgaSize);
	sys->GraficosCPC = usarCPC;
}

// Pinta la imagen de portada en pantalla y establece la paleta intro.
// Puede llamarse tanto desde muestraPresentacion() como desde
// repintaEstadoActual() cuando el estado actual es INTRO.
void Juego::pintaPortada()
{
	sys->setIntroPalette();
	UINT8 *romsVGA = &roms[0x24000-1-0x4000];
	UINT8 *screen  = romsVGA + 0x1ADF0;
	for (int j = 0; j < 200; j++)
		for (int i = 0; i < 320; i++)
			sys->setPixelIntro(i, j, *screen++);
}

// Repinta la pantalla completa usando la paleta y el contenido adecuados
// para el estado actual. Se llama tras un cambio de gráficos en caliente.
void Juego::repintaEstadoActual()
{
	switch (currentState)
	{
	case STATES::INTRO:
		pintaPortada();
		break;
	case STATES::SCROLL:
		// El pergamino se redibuja completamente en el siguiente tick;
		// solo necesitamos establecer la paleta correcta.
		sys->setGamePalette(1);
		break;
	case STATES::PLAY:
		sys->resetPalette();
		ReiniciaPantalla();
		motor->compruebaCambioPantalla(true);
		break;
	case STATES::ENDING:
		sys->setGamePalette(1);
		// El pergamino final se redibuja en el siguiente tick.
		break;
	default:
		// Menús y diálogos: paleta 2, área de juego limpia.
		sys->setGamePalette(2);
		ReiniciaPantalla();
		break;
	}
}

// Alterna entre gráficos VGA y CPC, guarda la preferencia y repinta.
void Juego::cambioCPC_VGA()
{
	aplicaGraficos(!sys->GraficosCPC);   // alterna y copia datos al buffer activo
	generaGraficosFlipeados();      // recalcula todos los sprites flipeados
	// La persistencia la gestiona automáticamente ConfigVar al asignar sys->GraficosCPC

	repintaEstadoActual();
}

/////////////////////////////////////////////////////////////////////////////
// menús
/////////////////////////////////////////////////////////////////////////////
bool Juego::menuCargar()
{
	if (loadMenu.isEmpty()) {
		loadMenu.clear();
		for (int i = 0; i < 7; ++i)
			loadMenu.add([this, i]() { return sys->slotDates[i]; }, [this, i]() {
				selectedSlot = i;
				if (activeGame) changeState(STATES::ASK_CONTINUE);
				else { logica->inicia(); cargar(i); changeState(STATES::PLAY); }
			});
		loadMenu.add([this]() { return textSave[sys->idioma]; }, [this]() { changeState(STATES::MENU); });
	}
	return loadMenu.tick(*marcador);
}

bool Juego::cargar(int slot)
{	
	return sys->loadSlot(slot, [&](std::ifstream& f){ f >> logica; });
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

void Juego::askExit()
{
	if (askExitMenu.isEmpty()) {
		askExitMenu.clear();
		askExitMenu.setOrientation(MenuOrientation::HORIZONTAL);
		askExitMenu.setMode(MenuMode::YESNO);
		askExitMenu.setPrompt([this]() { return continueQuestionText[sys->idioma]; });
		askExitMenu.add([this]() { return yesText[sys->idioma]; }, [this]() { sys->exitGame(); });
		askExitMenu.add([this]() { return noText[sys->idioma]; }, [this]() {
			if (activeGame) {
				changeState(STATES::PLAY);
				ReiniciaPantalla();
				activeGame = true;
				sys->setNormalSpeed();
			} else {
				changeState(STATES::SCROLL);
				ReiniciaPantalla();
			}
		});
	}
	askExitMenu.tick(*marcador);
}

void Juego::askForNewGame()
{
	if (askNewMenu.isEmpty()) {
		askNewMenu.clear();
		askNewMenu.setOrientation(MenuOrientation::HORIZONTAL);
		askNewMenu.setMode(MenuMode::YESNO);
		askNewMenu.setPrompt([this]() { return newGameQuestionText[sys->idioma]; });
		askNewMenu.add([this]() { return yesText[sys->idioma]; }, [this]() {
			logica->inicia();
			changeState(STATES::PLAY);
			ReiniciaPantalla();
		});
		askNewMenu.add([this]() { return noText[sys->idioma]; }, [this]() {
			changeState(STATES::PLAY);
			ReiniciaPantalla();
		});
	}
	askNewMenu.tick(*marcador);
}

void Juego::askToContinue()
{
	if (askContMenu.isEmpty()) {
		askContMenu.clear();
		askContMenu.setOrientation(MenuOrientation::HORIZONTAL);
		askContMenu.setMode(MenuMode::YESNO);
		askContMenu.setPrompt([this]() { return continueQuestionText[sys->idioma]; });
		askContMenu.add([this]() { return yesText[sys->idioma]; }, [this]() {
			logica->inicia();
			cargar(selectedSlot);
			changeState(STATES::PLAY);
			ReiniciaPantalla();
		});
		askContMenu.add([this]() { return noText[sys->idioma]; }, [this]() {
			changeState(STATES::PLAY);
			ReiniciaPantalla();
		});
	}
	askContMenu.tick(*marcador);
}

void Juego::save(int slot)
{
	sys->saveSlot(slot, [&](std::ofstream& f){ f << logica; });
}

bool Juego::menuGrabar()
{
	if (saveMenu.isEmpty()) {
		saveMenu.clear();
		for (int i = 0; i < 7; ++i)
			saveMenu.add([this, i]() { return sys->slotDates[i]; }, [this, i]() {
				save(i);
				changeState(STATES::PLAY);
				ReiniciaPantalla();
			});
		saveMenu.add([this]() { return textSave[sys->idioma]; }, [this]() { changeState(STATES::MENU); });
	}
	return saveMenu.tick(*marcador);
}

bool Juego::menuIdioma()
{
	if (langMenu.isEmpty()) {
		langMenu.clear();
		for (int i = 0; i < 8; ++i) {
			langMenu.add([this, i]() { return textLanguage[i]; }, [this, i]() {
				sys->idioma = i;
				seleccionado = 4;
				changeState(STATES::MENU);
			});
		}
	}
	return langMenu.tick(*marcador);
}

bool Juego::menu()
{
	if (mainMenu.isEmpty()) {
		mainMenu.clear();
		mainMenu.setAlignment(MenuAlignment::LEFT);
		// 0 Continuar
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][0]; },
			[this]() {
				changeState(STATES::PLAY);
				activeGame = true;
			},
			[this]() { return activeGame; }
		);

		// 1 Nueva partida
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][1]; },
			[this]() {
				if (!activeGame) {
					changeState(STATES::SCROLL);
					marcador->limpiaAreaMarcador();
					ReiniciaPantalla();
					sys->setFastSpeed();
					sys->playSound(SONIDOS::Inicio);
				} else {
					changeState(STATES::ASK_NEW_GAME);
				}
			}
		);

		// 2 Cargar
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][2]; },
			[this]() {
				checkForSaveFiles();
				changeState(STATES::LOAD);
			}
		);

		// 3 Guardar
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][3]; },
			[this]() {
				checkForSaveFiles();
				changeState(STATES::SAVE);
			},
			[this]() { return activeGame; }
		);

		// 4 Configuración
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][4]; },
			[this]() { changeState(STATES::CONFIG); }
		);

		// 5 Idioma
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][5]; },
			[this]() {
				seleccionado = sys->idioma;
				changeState(STATES::LANGUAGE);
			}
		);

		// 6 Sonido ON/OFF — toggle rápido, estado visible en el texto
		mainMenu.add(
			[this]() {
				return std::string(principalMenuText[sys->idioma][6])
					+ (sys->mute ? " OFF " : " ON  ");
			},
			[this]() {
				sys->mute = !sys->mute; // ConfigVar persiste automáticamente
			}
		);

		// 7 Ayuda
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][7]; },
			[this]() { changeState(STATES::HELP); }
		);

		// 8 MAPA
		mainMenu.add(
			[this]() { return std::string(principalMenuText[sys->idioma][8]) + (modoInformacion ? " ON  " : " OFF "); },
			[this]() { modoInformacion=!modoInformacion; },
			[this]() { return estadoContenido == STATES::PLAY; }
		);

		mainMenu.fill();

		// 9 Salir
		mainMenu.add(
			[this]() { return principalMenuText[sys->idioma][9]; },
			[this]() { changeState(STATES::ASK_EXIT); }
		);
	}

	return mainMenu.tick(*marcador);
}

bool Juego::menuConfig()
{
	if (configMenu.isEmpty()) {
		configMenu.clear();
		configMenu.setAlignment(MenuAlignment::LEFT);
		// 0 Gráficos
		configMenu.add(
			[this]() { return configMenuText[sys->idioma][0]; },
			[this]() { changeState(STATES::CONFIG_GFX); }
		);

		// 1 Sonido
		configMenu.add(
			[this]() { return configMenuText[sys->idioma][1]; },
			[this]() { changeState(STATES::CONFIG_SND); }
		);

		configMenu.fill();

		// 9 Volver
		configMenu.add(
			[this]() { return configMenuText[sys->idioma][2]; },
			[this]() { changeState(STATES::MENU); }
		);
	}

	return configMenu.tick(*marcador);
}

bool Juego::menuConfigGfx()
{
	if (configGfxMenu.isEmpty()) {
		configGfxMenu.clear();
		configGfxMenu.setAlignment(MenuAlignment::LEFT);
		// 0 Modo VGA/CPC — toggle, estado visible en el texto
		configGfxMenu.add(
			[this]() {
				return std::string(configGfxMenuText[sys->idioma][0])
					+ (sys->GraficosCPC ? " CPC " : " VGA ");
			},
			[this]() {
				cambioCPC_VGA();
				if (activeGame) changeState(STATES::PLAY);
			},
			[this]() {
				return estadoContenido != STATES::INTRO && estadoContenido != STATES::SCROLL;
			}
		);

		// 1 Filtro — no implementado
		configGfxMenu.add(
			[this]() {
				// TODO: faltan traducciones por idioma
				switch(sys->filtro) {
				case 0: return std::string(configGfxMenuText[sys->idioma][1]) + " NINGUNO"; break;
				case 1: return std::string(configGfxMenuText[sys->idioma][1]) + " XBR    "; break;
//				case 2: return std::string(configGfxMenuText[sys->idioma][1]) + " HQX    "; break;
				}
				return std::string("VALOR DE FILTRO CONFIGUDADO INVALIDO");
			},
			[this]() {
//				SDL_Log("menuConfigGfx: filtro no implementado");
//				sys->filtro=(sys->filtro+1)%3; // ciclar 0-2
				sys->filtro=(sys->filtro+1)%2; // ciclar 0-1 mientras no está implementado HQX
			},
			[this]() { return sys->useWebGL; }
		);

		// 2 Paleta — no implementado
		configGfxMenu.add(
			[this]() {
				//return std::string(configGfxMenuText[sys->idioma][2]) + " ESTANDAR ";
				// TODO: faltan traducciones por idioma
				switch (sys->paletaEfecto) {
				 case 0: return std::string(configGfxMenuText[sys->idioma][2]) + " ESTANDAR     "; break;
				 case 1: return std::string(configGfxMenuText[sys->idioma][2]) + " GRIS         "; break;
				 case 2: return std::string(configGfxMenuText[sys->idioma][2]) + " FÓSFORO VERDE"; break;
				 case 3: return std::string(configGfxMenuText[sys->idioma][2]) + " FÓSFORO ÁMBAR"; break;
				}
				return std::string("VALOR DE PALETA CONFIGUDADO INVALIDO");
			},
			[this]() {
				SDL_Log("menuConfigGfx: pruebas paleta ");
				sys->paletaEfecto = (sys->paletaEfecto + 1) % 4; // ciclar 0-3
				sys->resetPalette(); // regenera con nuevo efecto

			},
			[this]() { return true; }
		);

		configGfxMenu.fill();

		// 9 Volver
		configGfxMenu.add(
			[this]() { return configGfxMenuText[sys->idioma][3]; },
			[this]() { changeState(STATES::CONFIG); }
		);
	}

	return configGfxMenu.tick(*marcador);
}

bool Juego::menuConfigSnd()
{
	if (configSndMenu.isEmpty()) {
		configSndMenu.clear();
		configSndMenu.setAlignment(MenuAlignment::LEFT);
		// 0 Mute ON/OFF
		configSndMenu.add(
			[this]() {
				return std::string(configSndMenuText[sys->idioma][0])
					+ (sys->mute ? " OFF " : " ON  ");
			},
			[this]() {
				sys->mute = !sys->mute; // ConfigVar persiste automáticamente
			}
		);

		// 1 Volumen música — no implementado
		configSndMenu.add(
			[this]() {
				return std::string(configSndMenuText[sys->idioma][1]) + "  ";
			},
			[this]() {
				SDL_Log("menuConfigSnd: volumen musica no implementado");
			},
			[this]() { return false; }
		);

		// 2 Volumen efectos — no implementado
		configSndMenu.add(
			[this]() {
				return std::string(configSndMenuText[sys->idioma][2]) + "  ";
			},
			[this]() {
				SDL_Log("menuConfigSnd: volumen efectos no implementado");
			},
			[this]() { return false; }
		);

		configSndMenu.fill();
		// 9 Volver
		configSndMenu.add(
			[this]() { return configSndMenuText[sys->idioma][3]; },
			[this]() { changeState(STATES::CONFIG); }
		);
	}

	return configSndMenu.tick(*marcador);
}

bool Juego::menuAyuda()
{
	if (helpMenu.isEmpty()) {
		helpMenu.clear();
		helpMenu.setAlignment(MenuAlignment::LEFT);
		// 0 Pergamino de introducción con texto del manual
		helpMenu.add(
			[this]() { return helpMenuText[sys->idioma][0]; },
			[this]() {
				sys->setFastSpeed();
				changeState(STATES::HELP_INTRODUCCION); 
				pergamino->reset();
			},
			[this]() { return true; }
		);

		// 1 Manejo del teclado
		helpMenu.add(
			[this]() { return helpMenuText[sys->idioma][1]; },
			[this]() {
				sys->setFastSpeed();
				changeState(STATES::HELP_MANEJO_PERGAMINO);
				pergamino->reset();
			},
			[this]() { return true; }
		);
		// 2 Ayudas y mejoras
		helpMenu.add(
			[this]() { return helpMenuText[sys->idioma][2]; },
			[this]() {
				changeState(STATES::HELP_AYUDAS);
			},
			[this]() { return true; }
		);

		// 3 ayuda cámaras
		helpMenu.add(
			[this]() { return helpMenuText[sys->idioma][3]; },
			[this]() {
				changeState(STATES::HELP_CAMARAS);
			},
			[this]() { return true; }
		);

		// 4 Referencias
		helpMenu.add(
			[this]() { return helpMenuText[sys->idioma][4]; },
			[this]() {
				changeState(STATES::HELP_REFERENCIAS);
				sys->setFastSpeed();
				pergamino->reset();
			},
			[this]() { return true; }
		);

		// 5 Introducción (pergamino de inicio)
		helpMenu.add(
			[this]() { return helpMenuText[sys->idioma][5]; },
			[this]() {
				changeState(STATES::SCROLL);
				sys->setFastSpeed();
				sys->playSound(SONIDOS::Inicio);
			}
		);

		helpMenu.fill();

		// 9 Volver
		helpMenu.add(
			[this]() { return helpMenuText[sys->idioma][6]; },
			[this]() { changeState(STATES::MENU); }
		);
	}

	return helpMenu.tick(*marcador);
}

bool Juego::helpIntroduccion() {
	pergamino->muestraTexto(Pergamino::pergaminoIntroduccion[sys->idioma]);
	if (pergamino->finished)
	{
		sys->setNormalSpeed();
		changeState(Abadia::STATES::HELP);
	}
	return true;
}

bool Juego::helpManejoPergamino() {
	pergamino->muestraTexto(Pergamino::pergaminoManejo[sys->idioma]);
	if (pergamino->finished)
	{
		sys->setNormalSpeed();
		changeState(Abadia::STATES::HELP_MANEJO);
	}
	return true;
}

bool Juego::helpManejo() {
	if (BUTTON_YES) {
		changeState(STATES::MENU);
	} else {
		marcador->imprimeFrase(helpManejoText[sys->idioma][0], 8, 16+(0*16),4, 0);
		for (int i=1;i<9;i++)
		{
			marcador->imprimeFrase(helpManejoText[sys->idioma][i], 8, 16+(i*16),0, 4);
		}
	}
	return true;
}

bool Juego::helpAyudas() {
	if (BUTTON_YES) {
		changeState(STATES::HELP);
	} else {
		marcador->imprimeFrase(helpAyudasText[sys->idioma][0], 8, 16+(0*16),4, 0);
		for (int i=1;i<9;i++)
		{
			marcador->imprimeFrase(helpAyudasText[sys->idioma][i], 8, 16+(i*16),0, 4);
		}
	}
	return true;
}

bool Juego::helpCamaras() {
	if (BUTTON_YES) {
		changeState(STATES::HELP);
	} else {
		marcador->imprimeFrase(helpCamarasText[sys->idioma][0], 0, 16+(0*16),4, 0);
		for (int i=1;i<9;i++)
		{
			marcador->imprimeFrase(helpCamarasText[sys->idioma][i], 0, 16+(i*16),0, 4);
		}
	}
	return true;
}

bool Juego::helpReferencias() {
	pergamino->muestraTexto(Pergamino::pergaminoReferencias[sys->idioma]);
	if (pergamino->finished)
	{
		sys->setNormalSpeed();
		changeState(Abadia::STATES::HELP);
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// método principal del juego
/////////////////////////////////////////////////////////////////////////////
void Juego::preRun()
{	
	marcador->limpiaAreaMarcador();
	creaEntidadesJuego();

	// Los gráficos correctos (VGA o CPC) ya fueron copiados al buffer activo
	// en el constructor por aplicaGraficos(). Solo generamos los flipeados.
	generaGraficosFlipeados();

	motor->personaje = personajes[0];
	infoJuego->inicia();
	logica->despHabitacionEspejo();
	logica->inicia();

	marcador->limpiaAreaMarcador();
	logica->inicia();
	ReiniciaPantalla();
}

void Juego::changeState(Abadia::STATES newState)
{
	if (newState == currentState) return;
	marcador->limpiaAreaMarcador();  // solo el marcador, no ReiniciaPantalla completo

	switch (currentState) {
		case STATES::SCROLL:  sys->stopSound(Abadia::SONIDOS::Inicio); break;
		case STATES::ENDING:  sys->stopSound(Abadia::SONIDOS::Final);  break;
		default: break;
	}

	switch (newState) {
		case STATES::PLAY:
			if (!activeGame) activeGame = true;
			pausaPorEstarEnMenus = false;
			if (currentState != STATES::SCROLL && currentState != STATES::ENDING)
				sys->resumeSounds();
			ReiniciaPantalla();  
			break;
		case STATES::INTRO:
			pausaPorEstarEnMenus = true;
			sys->pauseSounds();
			break;
		case STATES::HELP:
		case STATES::HELP_INTRODUCCION:
		case STATES::HELP_MANEJO_PERGAMINO:
		case STATES::HELP_MANEJO:
		case STATES::HELP_AYUDAS:
		case STATES::HELP_CAMARAS:
		case STATES::HELP_REFERENCIAS:
		case STATES::CONFIG:
		case STATES::CONFIG_GFX:
		case STATES::CONFIG_SND:
		case STATES::LANGUAGE:
		case STATES::MENU:
		case STATES::LOAD:
		case STATES::SAVE:
		case STATES::ASK_NEW_GAME:
		case STATES::ASK_CONTINUE:
		case STATES::ASK_EXIT:
			pausaPorEstarEnMenus = true;
			sys->pauseSounds();
			sys->setGamePalette(2);
			limpiaAreaJuego(4);
			marcador->limpiaAreaMarcador();  // solo el marcador, no ReiniciaPantalla completo
			break;
		case STATES::SCROLL:
		case STATES::ENDING:
			pausaPorEstarEnMenus = true;
			sys->setGamePalette(1);
			pergamino->reset();   //  <- fuerza redibujado limpio desde muestraTexto()
			break;
	}

	switch (newState) {
		case STATES::INTRO:
		case STATES::SCROLL:
		case STATES::PLAY:
		case STATES::ENDING:
			estadoContenido = newState;
			break;
		default: break;
	}

	currentState = newState;
}

void Juego::run()
{
	if (sys->pad.map) {
		modoInformacion=!modoInformacion;
		limpiaAreaJuego(12);
		motor->compruebaCambioPantalla(true);
	}
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
// colores compatibles con el modo en que
// el mapa se superpone al juego
// y el fondo no se mezcla con el mapa
// tanto en VGA como en CPC
// 4
// 16 regulero
// 19 regulero
// 20 feo
// 21 feo
	int fondo=4;
// esta es es el margen izquierdo de la zona de juego
// que en la intro (imagen de portada) si se escribe
// tambien se escribe en el mapa
	sys->fillMode1Rect(0, 0, 32, 160, fondo);
// esta es la parte de la zona de juego
	sys->fillMode1Rect(32, 0, 256, 160, color);
// esta es es el margen derecho de la zona de juego
// que en la intro (imagen de portada) si se escribe
// tambien se escribe en el mapa
	sys->fillMode1Rect(0, 0, 32, 160, fondo);
	sys->fillMode1Rect(32 + 256, 0, 32, 160, fondo);	
// pero esto no borra la zona del marcador
// que se tendría que borrar con limpiaAreaMarcador
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

		int data = 0 ;

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

/////////////////////////////////////////////////////////////////////////////
// métodos para mostrar distintas pantallas
/////////////////////////////////////////////////////////////////////////////
void Juego::muestraPresentacion()
{
// Pintamos la portada cada frame (la paleta intro puede haberse
// sobreescrito si se vuelve aquí desde otro estado).
	pintaPortada();
	if (BUTTON_YES)
	{
		changeState(Abadia::STATES::MENU);
	}
}

void Juego::muestraIntroduccion()
{
	pergamino->muestraTexto(Pergamino::pergaminoInicio[sys->idioma]);
	if (pergamino->finished)
	{
		sys->setNormalSpeed();
		changeState(Abadia::STATES::PLAY);
		// changeState ya gestiona paleta, marcador y sonidos.
	}
}

void Juego::muestraFinal()
{
	sys->playSound(Abadia::SONIDOS::Final,true);
	pergamino->muestraTexto(Pergamino::pergaminoFinal[sys->idioma]);
}

bool Juego::muestraPantallaFinInvestigacion()
{
	std::string porcentaje[8] = {
		"XX POR CIENTO DE ",
		"XX  PER  CENT ",
		"XX POR CENTO DA ",
		"XX PER CENT DE ",
		"XX POR CENTO DA ",
		"XX PER CENTO ",
		"XX  PER  CENT ",
		"XX POR CENTO DA "
	};
	if (!logica->haFracasado) {return false;}

	laLogica->numPersonajeCamara = 0x80;

	if (elGestorFrases->mostrandoFrase) {return false;}

	limpiaAreaJuego(0);

	int porc = logica->calculaPorcentajeMision();

	porc = (porc < 0) ? 0 : (porc > 100 ? 100 : porc);
	porcentaje[sys->idioma][0] = ((porc/10) % 10) + 0x30;
	porcentaje[sys->idioma][1] = (porc % 10) + 0x30;

	int x = 0;
	x = (320 - (int)frase1[sys->idioma].length()*8) >> 1;
	marcador->imprimeFrase(frase1[sys->idioma], x, 32, 4, 0);
	x = (320 - (int)porcentaje[sys->idioma].length()*8) >> 1;
	marcador->imprimeFrase(porcentaje[sys->idioma], x, 48, 4, 0);
	x = (320 - (int)frase3[sys->idioma].length()*8) >> 1;
	marcador->imprimeFrase(frase3[sys->idioma], x, 64, 4, 0);
	x = (320 - (int)frase4[sys->idioma].length()*8) >> 1;
	marcador->imprimeFrase(frase4[sys->idioma], x, 128, 4, 0);

	if (sys->pad.action || sys->pad.confirm || sys->pad.cancel)
	{
		changeState(Abadia::STATES::INTRO);
		// changeState INTRO no pinta la portada; se pintará en el
		// siguiente tick de muestraPresentacion().
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

// ----------------------------------------------------------------------------
// checkForSaveFiles en Juego.cpp queda así (solo sincroniza con sys si es necesario)
// ----------------------------------------------------------------------------
void Juego::checkForSaveFiles()
{
	// sys ya gestiona las fechas de los slots automáticamente mediante ConfigVar y saveSlot/loadSlot.
	// Los menús leen directamente sys->slotDates[i]. Esta función se mantiene por compatibilidad de llamadas.
}
