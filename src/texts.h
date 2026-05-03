#ifndef __TEXTS_H__
#define __TEXTS_H__

#include <string>

enum LANGUAGE
{	
	CASTELLANO,
	ENGLISH,
	PORTUGUES_BRASIL,
	CATALAN,
	GALLEGO,
	ITALIANO,
	FINLANDES,
	PORTUGUES, // Europeo
	
	TOTAL_LANGUAGES
};

// Usamos literales UTF-8 explícitos para máxima compatibilidad multiplataforma
// (GCC, Clang, Emscripten, etc. los soportan bien)
/*
const std::string principalMenuText[TOTAL_LANGUAGES][10] =
{
	{	// Castellano
		u8"0 NUEVA PARTIDA",
		u8"1 CARGAR",
		u8"2 GUARDAR",
		u8"3 IDIOMA",
		u8"4 CONTINUAR",
		u8"5 GRÁFICOS",
		u8"6 SALIR",
		u8"7 INTRO",
		u8"8 PERGAMINO INICIAL",
		u8"9 SONIDO"

	},
	{	// English
		u8"NEW GAME",
		u8"LOAD",
		u8"SAVE",
		u8"LANGUAGE",
		u8"CONTINUE",
		u8"GRAPHICS VGA-CPC",
		u8"EXIT",
		u8"7 INTRO",
		u8"8 INITIAL SCROLL",
		u8"9 SOUND"
	},
	{	// Portugués Brasil
		u8"NOVO JOGO",
		u8"CARREGAR",
		u8"SALVAR",
		u8"IDIOMA",
		u8"CONTINUAR",
		u8"GRAPHICS VGA-CPC",
		u8"SAIR",
		u8"7 INTRO",
		u8"8 INITIAL SCROLL",
		u8"9 SOUND"
	},
	{	// Catalán
		u8"NOU JOC",
		u8"CARREGAR",
		u8"DESA",
		u8"IDIOMA",
		u8"CONTINUA",
		u8"GRAPHICS VGA-CPC",
		u8"SORTIR",
		u8"7 INTRO",
		u8"8 INITIAL SCROLL",
		u8"9 SOUND"
	},
	{	// Gallego
		u8"NOVO XOGO",
		u8"CARGAR",
		u8"GARDAR",
		u8"LINGUA",
		u8"CONTINUAR",
		u8"GRAPHICS VGA-CPC",
		u8"SAIR",
		u8"7 INTRO",
		u8"8 INITIAL SCROLL",
		u8"9 SOUND"
	},
	{	// Italiano
		u8"NUOVO GIOCO",
		u8"CARICA",
		u8"SALVA",
		u8"LINGUA",
		u8"CONTINUA",
		u8"GRAPHICS VGA-CPC",
		u8"ESCI",
		u8"7 INTRO",
		u8"8 INITIAL SCROLL",
		u8"9 SOUND"
	},	
	{	// Finlandés
		u8"UUSI PELI",
		u8"LATAA",
		u8"TALLENNA",
		u8"KIELI",
		u8"JATKA",
		u8"GRAPHICS VGA-CPC",
		u8"POISTU",
		u8"7 INTRO",
		u8"8 INITIAL SCROLL",
		u8"9 SOUND"
	},
	{	// Portugués Europeo
		u8"NOVO JOGO",
		u8"CARREGAR",
		u8"GUARDAR",
		u8"IDIOMA",
		u8"CONTINUAR",
		u8"GRAPHICS VGA-CPC",
		u8"SAIR",
		u8"7 INTRO",
		u8"8 INITIAL SCROLL",
		u8"9 SOUND"
	}
};
*/

// -----------------------------------------------------------------
// Menú principal — 9 items, el número va en el texto
// -----------------------------------------------------------------
constexpr const char* principalMenuText[8][10] = {
    { // 0 Castellano
        "0 CONTINUAR",
        "1 NUEVA PARTIDA",
        "2 CARGAR",
        "3 GUARDAR",
        "4 CONFIGURACION",
        "5 IDIOMA",
        "6 SONIDO",
        "7 AYUDA",
	"8 MAPA",
        "9 SALIR"
    },
    { // 1 English
        "0 CONTINUE",
        "1 NEW GAME",
        "2 LOAD",
        "3 SAVE",
        "4 SETTINGS",
        "5 LANGUAGE",
        "6 SOUND",
        "7 HELP",
	"8 MAP",
        "9 EXIT"
    },
    { // 2 Portugues Brasil
        "0 CONTINUAR",
        "1 NOVO JOGO",
        "2 CARREGAR",
        "3 GUARDAR",
        "4 CONFIGURACAO",
        "5 IDIOMA",
        "6 SOM",
        "7 AJUDA",
	"8 MAPA",
        "9 SAIR"
    },
    { // 3 Catalan
        "0 CONTINUAR",
        "1 NOVA PARTIDA",
        "2 CARREGAR",
        "3 GUARDAR",
        "4 CONFIGURACIO",
        "5 IDIOMA",
        "6 SO",
        "7 AJUDA",
	"8 MAPA",
        "9 SORTIR"
    },
    { // 4 Gallego
        "0 CONTINUAR",
        "1 NOVA PARTIDA",
        "2 CARGAR",
        "3 GARDAR",
        "4 CONFIGURACION",
        "5 IDIOMA",
        "6 SON",
        "7 AXUDA",
	"8 MAP",
        "9 SAÍR"
    },
    { // 5 Italiano
        "0 CONTINUA",
        "1 NUOVA PARTITA",
        "2 CARICA",
        "3 SALVA",
        "4 IMPOSTAZIONI",
        "5 LINGUA",
        "6 SUONO",
        "7 AIUTO",
	"8 MAPPA",
        "9 ESCI"
    },
    { // 6 Fines
        "0 JATKA",
        "1 UUSI PELI",
        "2 LATAA",
        "3 TALLENNA",
        "4 ASETUKSET",
        "5 KIELI",
        "6 AANI",
        "7 OHJE",
	"8 KARTTA",
        "9 LOPETA"
    },
    { // 7 Portugues
        "0 CONTINUAR",
        "1 NOVO JOGO",
        "2 CARREGAR",
        "3 GUARDAR",
        "4 CONFIGURACAO",
        "5 IDIOMA",
        "6 SOM",
        "7 AJUDA",
	"8 MAPA",
        "9 SAIR"
    }
};

// -----------------------------------------------------------------
// Menú configuración — 3 items visibles + volver
// -----------------------------------------------------------------
constexpr const char* configMenuText[8][3] = {
    { "0 GRAFICOS", "1 SONIDO", "9 VOLVER" },
    { "0 GRAPHICS", "1 SOUND",  "9 BACK"   },
    { "0 GRAFICOS", "1 SOM",    "9 VOLTAR"  },
    { "0 GRAFICS",  "1 SO",     "9 TORNAR"  },
    { "0 GRAFICOS", "1 SON",    "9 VOLVER"  },
    { "0 GRAFICA",  "1 SUONO",  "9 INDIETRO"},
    { "0 GRAFIIKKA","1 AANI",   "9 TAKAISIN"},
    { "0 GRAFICOS", "1 SOM",    "9 VOLTAR"  }
};

// -----------------------------------------------------------------
// Menú gráficos — 4 items: VGA/CPC, filtro, paleta, volver
// -----------------------------------------------------------------
constexpr const char* configGfxMenuText[8][4] = {
    { "0 MODO",   "1 FILTRO", "2 PALETA", "9 VOLVER"   },
    { "0 MODE",   "1 FILTER", "2 PALETTE","9 BACK"     },
    { "0 MODO",   "1 FILTRO", "2 PALETA", "9 VOLTAR"   },
    { "0 MODE",   "1 FILTRE", "2 PALETA", "9 TORNAR"   },
    { "0 MODO",   "1 FILTRO", "2 PALETA", "9 VOLVER"   },
    { "0 MODO",   "1 FILTRO", "2 PALETTE","9 INDIETRO" },
    { "0 TILA",   "1 FILTTERI","2 PALETTI","9 TAKAISIN" },
    { "0 MODO",   "1 FILTRO", "2 PALETA", "9 VOLTAR"   }
};

// -----------------------------------------------------------------
// Menú sonido — 4 items: mute, vol música, vol efectos, volver
// -----------------------------------------------------------------
constexpr const char* configSndMenuText[8][4] = {
    { "0 SONIDO", "1 MUSICA",  "2 EFECTOS", "9 VOLVER"   },
    { "0 MUTE",     "1 MUSIC",   "2 EFFECTS", "9 BACK"     },
    { "0 MUDO",     "1 MUSICA",  "2 EFEITOS", "9 VOLTAR"   },
    { "0 MUT",      "1 MUSICA",  "2 EFECTES", "9 TORNAR"   },
    { "0 MUDO",     "1 MUSICA",  "2 EFECTOS", "9 VOLVER"   },
    { "0 MUTO",     "1 MUSICA",  "2 EFFETTI", "9 INDIETRO" },
    { "0 MYKISTA",  "1 MUSIIKKI","2 TEHOSTEET","9 TAKAISIN" },
    { "0 MUDO",     "1 MUSICA",  "2 EFEITOS", "9 VOLTAR"   }
};

// -----------------------------------------------------------------
// Menú ayuda — 7 items: 6 secciones + volver
// -----------------------------------------------------------------
constexpr const char* helpMenuText[8][7] = {
    { "0 INTRODUCCION", "1 MANEJO DEL TECLADO", "2 CAMARAS",
      "3 INTRODUCCION", "4 REFERENCIAS", "5 CREDITOS", "9 VOLVER" },
    { "0 INTRODUCTION",  "1 KEYBOARD CONTROLS", "2 CAMERAS",
      "3 INTRODUCTION","4 REFERENCES",   "5 CREDITS",  "9 BACK"   },
    { "0 INTRODUCAO", "1 CONTROLES DO TECLADO", "2 CAMERAS",
      "4 INTRODUCAO",  "4 REFERENCIAS",  "5 CREDITOS", "9 VOLTAR" },
    { "0 INTRODUCCIO",  "1 ÚS DE TECLAT", "2 CAMERAS",
      "3 INTRODUCCIO", "4 REFERENCIES",  "5 CREDITS",  "9 TORNAR" },
    { "0 INTRODUCION", "1 MANEXO DO TECLADO", "2 CAMARAS",
      "3 INTRODUCION", "4 REFERENCIAS",  "5 CREDITOS", "9 VOLVER" },
    { "0 INTRODUZIONE", "1 USO DELLA TASTIERA",   "2 CAMERE",
      "3 INTRODUZIONE","4 RIFERIMENTI",  "5 CREDITI",  "9 INDIETRO"},
    { "0 JOHDANTO",  "1 NÄPPÄIMISTÖN KÄYTTÖ",  "2 KAMERAT",
      "3 JOHDANTO",   "4 VIITTEET",      "5 ANSIOLUETTELO","9 TAKAISIN"},
    { "0 INTRODUCAO", "1 UTILIZAÇÃO DO TECLADO", "2 CAMERAS",
      "3 INTRODUCAO",  "4 REFERENCIAS",  "5 CREDITOS", "9 VOLTAR" }
};

constexpr const char* helpManejoText[TOTAL_LANGUAGES][9] = {
	{ // 0 Castellano 
			"           MANEJO DEL TECLADO          ",
			"CURSOR ARRIBA: MOVER A GUILLERMO",
			"CURSOR ABAJO: MOVER A ADSO",
			"CURSOR IZQUIERDA: GIRAR A LA IZQUIERDA",
			"CURSOR DERECHA: GIRAR A LA DERECHA",
			"ESPACIO: DEJAR OBJETOS",
			"SUPR: PAUSA",
			"",
			"    -PULSA ESPACIO PARA CONTINUAR-" 
	},
		{ // 1 INGLES
			"                 KEYBOARD              ",
			"UP ARROW: MOVE WILLIAM",
			"DOWN ARROW: MOVE ADSO",
			"LEFT ARROW: TURN LEFT",
			"RIGHT ARROW: TURN RIGHT",
			"SPACE: DROP OBJECTS",
			"SUPR: PAUSE",
			"",
			"           -PRESS SPACE-          "
		},
		{ // 2 PORTUGUES BRASIL
			"           MANEJO DEL TECLADO          ",
			"CURSOR ARRIBA: MOVER A GUILLERMO",
			"CURSOR ABAJO: MOVER A ADSO",
			"CURSOR IZQUIERDA: GIRAR A LA IZQUIERDA",
			"CURSOR DERECHA: GIRAR A LA DERECHA",
			"ESPACIO: DEJAR OBJETOS",
			"SUPR: PAUSA",
			"",
		},
		{ // 3 CATALAN
			"           MANEJO DEL TECLADO          ",
			"CURSOR ARRIBA: MOVER A GUILLERMO",
			"CURSOR ABAJO: MOVER A ADSO",
			"CURSOR IZQUIERDA: GIRAR A LA IZQUIERDA",
			"CURSOR DERECHA: GIRAR A LA DERECHA",
			"ESPACIO: DEJAR OBJETOS",
			"SUPR: PAUSA",
			"",
		},
		{ // 4 GALLEGO
			"           MANEJO DEL TECLADO          ",
			"CURSOR ARRIBA: MOVER A GUILLERMO",
			"CURSOR ABAJO: MOVER A ADSO",
			"CURSOR IZQUIERDA: GIRAR A LA IZQUIERDA",
			"CURSOR DERECHA: GIRAR A LA DERECHA",
			"ESPACIO: DEJAR OBJETOS",
			"SUPR: PAUSA",
			"",
		},
		{ // 5 ITALIANO
			"           MANEJO DEL TECLADO          ",
			"CURSOR ARRIBA: MOVER A GUILLERMO",
			"CURSOR ABAJO: MOVER A ADSO",
			"CURSOR IZQUIERDA: GIRAR A LA IZQUIERDA",
			"CURSOR DERECHA: GIRAR A LA DERECHA",
			"ESPACIO: DEJAR OBJETOS",
			"SUPR: PAUSA",
			"",
		},
		{ // 6 FINES
			"                 KEYBOARD              ",
			"UP ARROW: MOVE WILLIAM",
			"DOWN ARROW: MOVE ADSO",
			"LEFT ARROW: TURN LEFT",
			"RIGHT ARROW: TURN RIGHT",
			"SPACE: DROP OBJECTS",
			"SUPR: PAUSE",
			"",
		},
		{ // 7 PORTUGUES
			"           MANEJO DEL TECLADO          ",
			"CURSOR ARRIBA: MOVER A GUILLERMO",
			"CURSOR ABAJO: MOVER A ADSO",
			"CURSOR IZQUIERDA: GIRAR A LA IZQUIERDA",
			"CURSOR DERECHA: GIRAR A LA DERECHA",
			"ESPACIO: DEJAR OBJETOS",
			"SUPR: PAUSA",
			"",
		}
};

const std::string menuText[TOTAL_LANGUAGES][9] = 
{ 
	{ // Castellano
		u8"IDIOMA",
		u8"CARGAR PARTIDA",
		u8"GUARDAR PARTIDA",
		u8"GRÁFICOS VGA-CPC",
		u8"AYUDA",
		u8"TUTORIAL",
		u8"REINICIAR",
		u8"SONIDO",
		u8"JUGAR"
	},
	{ // Inglés
		u8"LANGUAGE",
		u8"LOAD GAME",
		u8"SAVE GAME",
		u8"GRAPHICS VGA-CPC",
		u8"HELP",
		u8"TUTORIAL",
		u8"RESTART",
		u8"SOUND",
		u8"PLAY"
	},
	{ // Portugués Brasil
		u8"IDIOMA",
		u8"CARREGAR JOGO",
		u8"SALVAR JOGO",
		u8"GRÁFICOS VGA-CPC",
		u8"AJUDA",
		u8"TUTORIAL",
		u8"REINICIAR",
		u8"SOM",
		u8"JOGAR"
	},
	{ // Catalán
		u8"IDIOMA",
		u8"CARREGAR JOC",
		u8"DESA PARTIDA",
		u8"GRÀFICS VGA-CPC",
		u8"AJUDA",
		u8"TUTORIAL",
		u8"REINICIA",
		u8"SO",
		u8"JUGA"
	},
	{ // Gallego
		u8"LINGUA",
		u8"CARGAR XOGO",
		u8"GARDAR XOGO",
		u8"GRÁFICOS VGA-CPC",
		u8"AXUDA",
		u8"TUTORIAL",
		u8"REINICIAR",
		u8"SON",
		u8"XOGAR"
	},
	{ // Italiano
		u8"LINGUA",
		u8"CARICA GIOCO",
		u8"SALVA PARTITA",
		u8"GRAFICA VGA-CPC",
		u8"AIUTO",
		u8"TUTORIAL",
		u8"RICOMINCIA",
		u8"SUONO",
		u8"GIOCA"
	},
	{ // Finlandés
		u8"KIELI",
		u8"LATAA PELI",
		u8"TALLENNA PELI",
		u8"GRAFIIKAT VGA-CPC",
		u8"OHJE",
		u8"TUTORIAL",
		u8"ALOITA ALUSTA",
		u8"ÄÄNI",
		u8"PELAA"
	},
	{ // Portugués Europeo
		u8"IDIOMA",
		u8"CARREGAR JOGO",
		u8"GUARDAR JOGO",
		u8"GRÁFICOS VGA-CPC",
		u8"AJUDA",
		u8"TUTORIAL",
		u8"REINICIAR",
		u8"SOM",
		u8"JOGAR"
	}
};

const std::string textSave[TOTAL_LANGUAGES] = 
{
	u8"VOLVER",
	u8"BACK",
	u8"VOLTAR",
	u8"TORNAR",
	u8"VOLTAR",
	u8"INDIETRO",
	u8"TAKAISIN",
	u8"VOLTAR"
};

const std::string textLanguage[TOTAL_LANGUAGES] = 
{ 
	u8"CASTELLANO",
	u8"ENGLISH",
	u8"PORTUGUÊS (BRASIL)",
	u8"CATALÀ",
	u8"GALEGO",
	u8"ITALIANO",
	u8"SUOMI",
	u8"PORTUGUÊS"
};

const std::string newGameQuestionText[TOTAL_LANGUAGES] =
{
	u8"¿DESEAS INICIAR\nUNA NUEVA PARTIDA?",
	u8"ARE YOU SURE YOU WANT\nTO START A NEW GAME?",
	u8"TEM CERTEZA DE QUE QUER\nCOMEÇAR UM NOVO JOGO?",
	u8"ESTÀS SEGUR QUE VOLS\nCOMENÇAR UN NOU JOC?",
	u8"TES SEGURIDADE DE QUERER\nCOMEZAR UN XOGO NOVO?",
	u8"SEI SICURO DI VOLER\nINIZIARE UNA NUOVA PARTITA?",
	u8"OLETKO VARMA, ETTÄ HALUAT\nALOITTAA UUDEN PELIN?",
	u8"TEM A CERTEZA DE QUE QUER\nCOMEÇAR UM NOVO JOGO?"
};

const std::string continueQuestionText[TOTAL_LANGUAGES] =
{
	u8"¿DESEAS CONTINUAR?\nPERDERÁS TODO EL PROGRESO",
	u8"DO YOU WANT TO CONTINUE?\nYOU WILL LOSE ALL PROGRESS",
	u8"QUER CONTINUAR?\nVOCÊ PERDERÁ TODO O PROGRESSO",
	u8"VOLS CONTINUAR?\nPERDRÀS TOT EL PROGRÉS",
	u8"QUERES CONTINUAR?\nPERDERÁS TODO O PROGRESO",
	u8"VUOI CONTINUARE?\nPERDERAI TUTTI I PROGRESSI",
	u8"HALUATKO JATKAA?\nMENETÄT KAIKEN EDISTYKSEN",
	u8"QUER CONTINUAR?\nPERDERÁ TODO O PROGRESSO"
};

const std::string yesText[TOTAL_LANGUAGES] = 
{
	u8"SÍ",
	u8"YES",
	u8"SIM",
	u8"SÍ",
	u8"SÍ",
	u8"SI", // u8"SÌ", falta añadir el glifo de Ì en alphabet.h
	u8"JOO", // u8"KYLLÄ" demasiado largo y se superpone al texto del NO
	u8"SIM"
};

const std::string noText[TOTAL_LANGUAGES] = 
{
	u8"NO",
	u8"NO",
	u8"NÃO",
	u8"NO",
	u8"NON",
	u8"NO",
	u8"EI",
	u8"NÃO"
};

// === TEXTOS COMPLETOS: CÁMARAS ===
static const std::string textCameras[TOTAL_LANGUAGES][9] = 
{ 
	{ // Castellano
		u8"   USA ESTAS TECLAS DURANTE EL JUEGO   ",
		u8"1 MANTÉN PULSADO PARA SEGUIR AL ABAD",
		u8"2 MANTÉN PULSADO PARA SEGUIR A SEVERINO",
		u8"3 MANTÉN PULSADO PARA SEGUIR A MALAQUÍAS",
		u8"4 MANTÉN PULSADO PARA SEGUIR A BERENGARIO",
		u8"5 MANTÉN PULSADO PARA SEGUIR A JORGE",
		u8"6 MANTÉN PULSADO PARA SEGUIR A BERNARDO",
		u8"7 MANTÉN PULSADO PARA SEGUIR A ADSO",
		u8"    -PULSA ESPACIO PARA CONTINUAR-"
	},
	{ // Inglés
		u8"            USE THESE KEYS IN-GAME            ",
		u8"HOLD 1 TO FOLLOW THE ABBOT",
		u8"HOLD 2 TO FOLLOW SEVERINUS",
		u8"HOLD 3 TO FOLLOW MALACHI",
		u8"HOLD 4 TO FOLLOW BERENGAR",
		u8"HOLD 5 TO FOLLOW JORGE",
		u8"HOLD 6 TO FOLLOW BERNARD GUI",
		u8"HOLD 7 TO FOLLOW ADSO",
		u8"        -PRESS SPACE TO CONTINUE-"
	},
	{ // Portugués Brasil
		u8"   USE ESTAS TECLAS DURANTE O JOGO   ",
		u8"1 MANTENHA PRESSIONADO PARA SEGUIR O ABADE",
		u8"2 MANTENHA PRESSIONADO PARA SEGUIR SEVERINO",
		u8"3 MANTENHA PRESSIONADO PARA SEGUIR MALAQUIAS",
		u8"4 MANTENHA PRESSIONADO PARA SEGUIR BERENGÁRIO",
		u8"5 MANTENHA PRESSIONADO PARA SEGUIR JORGE",
		u8"6 MANTENHA PRESSIONADO PARA SEGUIR BERNARDO GUI",
		u8"7 MANTENHA PRESSIONADO PARA SEGUIR ADSO",
		u8"    -PRESSIONE ESPAÇO PARA CONTINUAR-"
	},
	{ // Catalán
		u8"   FES SERVIR AQUESTES TECLES DURANT EL JOC   ",
		u8"1 MANTÉN PREMUT PER SEGUIR L’ABAT",
		u8"2 MANTÉN PREMUT PER SEGUIR EN SEVERÍ",
		u8"3 MANTÉN PREMUT PER SEGUIR EN MALAQUIES",
		u8"4 MANTÉN PREMUT PER SEGUIR EN BERENGUER",
		u8"5 MANTÉN PREMUT PER SEGUIR EN JORDI",
		u8"6 MANTÉN PREMUT PER SEGUIR EN BERNAT GUI",
		u8"7 MANTÉN PREMUT PER SEGUIR EN ADSÓ",
		u8"    -PREM ESPAI PER CONTINUAR-"
	},
	{ // Gallego
		u8"   USA ESTAS TECLAS DURANTE O XOGO   ",
		u8"1 MANTÉN PREMIDO PARA SEGUIR AO ABADE",
		u8"2 MANTÉN PREMIDO PARA SEGUIR A SEVERINO",
		u8"3 MANTÉN PREMIDO PARA SEGUIR A MALAQUIÁS",
		u8"4 MANTÉN PREMIDO PARA SEGUIR A BERENGARIO",
		u8"5 MANTÉN PREMIDO PARA SEGUIR A XURXE",
		u8"6 MANTÉN PREMIDO PARA SEGUIR A BERNARDO GUI",
		u8"7 MANTÉN PREMIDO PARA SEGUIR A ADSO",
		u8"    -PREME ESPAZO PARA CONTINUAR-"
	},
	{ // Italiano
		u8"   USA QUESTI TASTI DURANTE IL GIOCO   ",
		u8"1 TIENTI PREMUTO PER SEGUIRE L’ABATE",
		u8"2 TIENTI PREMUTO PER SEGUIRE SEVERINO",
		u8"3 TIENTI PREMUTO PER SEGUIRE MALACHIA",
		u8"4 TIENTI PREMUTO PER SEGUIRE BERENGARIO",
		u8"5 TIENTI PREMUTO PER SEGUIRE GIORGE",
		u8"6 TIENTI PREMUTO PER SEGUIRE BERNARDO GUI",
		u8"7 TIENTI PREMUTO PER SEGUIRE ADSO",
		u8"    -PREMI SPAZIO PER CONTINUARE-"
	},
	{ // Finlandés
		u8"   KÄYTÄ NÄITÄ NÄPPÄIMIÄ PELIN AIKANA   ",
		u8"1 PIDÄ PAINETTUNA SEURATAKSES ABBOTTIA",
		u8"2 PIDÄ PAINETTUNA SEURATAKSES SEVERINUSTA",
		u8"3 PIDÄ PAINETTUNA SEURATAKSES MALAKIAA",
		u8"4 PIDÄ PAINETTUNA SEURATAKSES BERENGARIA",
		u8"5 PIDÄ PAINETTUNA SEURATAKSES JORGETA",
		u8"6 PIDÄ PAINETTUNA SEURATAKSES BERNARD GUI'TA",
		u8"7 PIDÄ PAINETTUNA SEURATAKSES ADSOA",
		u8"    -PAINA VÄLILYÖNTIÄ JATKAAKSESI-"
	},
	{ // Portugués Europeu
		u8"   USA ESTAS TECLAS DURANTE O JOGO   ",
		u8"1 MANTENHA PRESSIONADO PARA SEGUIR O ABADE",
		u8"2 MANTENHA PRESSIONADO PARA SEGUIR SEVERINO",
		u8"3 MANTENHA PRESSIONADO PARA SEGUIR MALAQUIAS",
		u8"4 MANTENHA PRESSIONADO PARA SEGUIR BERENGÁRIO",
		u8"5 MANTENHA PRESSIONADO PARA SEGUIR JORGE",
		u8"6 MANTENHA PRESSIONADO PARA SEGUIR BERNARDO GUI",
		u8"7 MANTENHA PRESSIONADO PARA SEGUIR ADSO",
		u8"    -PRESSIONE ESPAÇO PARA CONTINUAR-"
	}
};

// === TEXTOS COMPLETOS: MEJORAS ===
static const std::string textEnhancements[TOTAL_LANGUAGES][9] = 
{ 
	{ // Castellano
		u8"   USA ESTAS TECLAS DURANTE EL JUEGO   ",
		u8"G GUARDAR PARTIDA",
		u8"C CARGAR PARTIDA",
		u8"F2 CAMBIAR ENTRE GRÁFICOS VGA O CPC",
		u8"F3 PANTALLA COMPLETA",
		u8"F5 MOSTRAR MAPAS",
		u8"SUPR PAUSA",
		u8"",
		u8"    -PULSA ESPACIO PARA CONTINUAR-"
	},
	{ // Inglés
		u8"            KEYBOARD SHORTCUTS         ",
		u8"G SAVE GAME",
		u8"C LOAD GAME",
		u8"F2 SWITCH VGA/CPC GRAPHICS",
		u8"F3 TOGGLE FULLSCREEN",
		u8"F5 SHOW MAPS",
		u8"DEL PAUSE",
		u8"",
		u8"        -PRESS SPACE TO CONTINUE-"
	},
	{ // Portugués Brasil
		u8"   USE ESTAS TECLAS DURANTE O JOGO   ",
		u8"G SALVAR JOGO",
		u8"C CARREGAR JOGO",
		u8"F2 TROCAR GRÁFICOS VGA/CPC",
		u8"F3 TELA CHEIA",
		u8"F5 MOSTRAR MAPAS",
		u8"DEL PAUSAR",
		u8"",
		u8"    -PRESSIONE ESPAÇO PARA CONTINUAR-"
	},
	{ // Catalán
		u8"   FES SERVIR AQUESTES TECLES DURANT EL JOC   ",
		u8"G DESA PARTIDA",
		u8"C CARREGA PARTIDA",
		u8"F2 CANVIA ENTRE GRÀFICS VGA/CPC",
		u8"F3 PANTALLA COMPLETA",
		u8"F5 MOSTRA MAPES",
		u8"SUPR PAUSA",
		u8"",
		u8"    -PREM ESPAI PER CONTINUAR-"
	},
	{ // Gallego
		u8"   USA ESTAS TECLAS DURANTE O XOGO   ",
		u8"G GARDAR XOGO",
		u8"C CARGAR XOGO",
		u8"F2 CAMBIAR ENTRE GRÁFICOS VGA/CPC",
		u8"F3 PANTALLA COMPLETA",
		u8"F5 MOSTRAR MAPAS",
		u8"SUPR PAUSA",
		u8"",
		u8"    -PREME ESPAZO PARA CONTINUAR-"
	},
	{ // Italiano
		u8"   USA QUESTI TASTI DURANTE IL GIOCO   ",
		u8"G SALVA PARTITA",
		u8"C CARICA PARTITA",
		u8"F2 CAMBIA GRAFICA VGA/CPC",
		u8"F3 SCHERMO INTERO",
		u8"F5 MOSTRA MAPPE",
		u8"CANC PAUSA",
		u8"",
		u8"    -PREMI SPAZIO PER CONTINUARE-"
	},
	{ // Finlandés
		u8"   KÄYTÄ NÄITÄ NÄPPÄIMIÄ PELIN AIKANA   ",
		u8"G TALLENNA PELI",
		u8"C LATAA PELI",
		u8"F2 VAIHDA GRAFIIKKATYYPPIÄ (VGA/CPC)",
		u8"F3 KOKO NÄYTTÖ",
		u8"F5 NÄYTÄ KARTAT",
		u8"DEL TAUKO",
		u8"",
		u8"    -PAINA VÄLILYÖNTIÄ JATKAES-"
	},
	{ // Portugués Europeu
		u8"   USA ESTAS TECLAS DURANTE O JOGO   ",
		u8"G GUARDAR JOGO",
		u8"C CARREGAR JOGO",
		u8"F2 MUDAR GRÁFICOS VGA/CPC",
		u8"F3 ECRÃ COMPLETO",
		u8"F5 MOSTRAR MAPAS",
		u8"DEL PAUSA",
		u8"",
		u8"    -PRESSIONE ESPAÇO PARA CONTINUAR-"
	}
};

const std::string frase1[TOTAL_LANGUAGES] = {
	u8"HAS RESUELTO EL",
	u8"YOU HAVE SOLVED",
	u8"VOCÊ RESOLVEU",
	u8"HAS RESOLT EL",
	u8"RESOLVEUCHES O",
	u8"HAI RISOLTO IL",
	u8"OLET RATKAISSENA",
	u8"RESOLVEU O"
};

const std::string frase3[TOTAL_LANGUAGES] = {
	u8"LA INVESTIGACIÓN",
	u8"THE INVESTIGATION",
	u8"A INVESTIGAÇÃO",
	u8"LA INVESTIGACIÓ",
	u8"A INVESTIGACIÓN",
	u8"L’INDAGINE",
	u8"TUTKIMUKSEN",
	u8"A INVESTIGAÇÃO"
};

const std::string frase4[TOTAL_LANGUAGES] = {
	u8"PRESIONA CUALQUIER BOTÓN PARA REINICIAR",
	u8"PRESS ANY BUTTON TO RESTART",
	u8"PRESSIONE QUALQUER BOTÃO PARA REINICIAR",
	u8"PREMEU QUALSEVOL BOTÓ PER REINICIAR",
	u8"PREME CALQUERA BOTÓN PARA REINICIAR",
	u8"PREMI UN TASTO QUALSIASI PER RICOMINCIARE",
	u8"PAINA MITÄ TAHANSA PAINIKETTA UUDENNUKSEEN",
	u8"PRESSIONE QUALQUER BOTÃO PARA REINICIAR"
};

#endif
