#include "abbey.h"
#include "DskReader.h"
#include "GestorFrases.h"
#include "system.h"
#include <cstdio>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

// -------------------------------------------------------------------------
// Constructor / destructor
// -------------------------------------------------------------------------

Abbey::Abbey()
	:_game(nullptr)
	, _fileLoader(new FileLoader())
	 , _romsPtr(nullptr)
{
}

Abbey::~Abbey()
{
	delete _game;
	delete[] _romsPtr;
	delete _fileLoader;

	for (auto *e : _gameFiles) {
		e->free();
		delete e;
	} 
}

// -------------------------------------------------------------------------
// init
// -------------------------------------------------------------------------

bool Abbey::init()
{
    // Registra los ficheros necesarios (antes en AbadiaDriver::createGameDataEntities)
    auto *roms = new GameDataEntity(MIXED, "Code + Graphics + Sound");
    roms->addFile(new GameFile("abadia.dsk", 0x00000, 0x27400, 0xd37cf8e7, 0));
    _gameFiles.push_back(roms);

    auto *gfxVGA = new GameDataEntity(GRAPHICS, "Graphics VGA + Palette");
    gfxVGA->addFile(new GameFile("GraficosVGA", 0, 174065, 0, 0));
    _gameFiles.push_back(gfxVGA);

    auto *gfxCPC = new GameDataEntity(GRAPHICS, "Graphics CPC + Palette");
    gfxCPC->addFile(new GameFile("GraficosCPC", 0, 174065, 0, 0));
    _gameFiles.push_back(gfxCPC);

    if (!loadFiles())
        return false;

    filesLoaded();
    deallocateFilesMemory();
    finishInit();

    // preRun: crea entidades del juego y arranca el estado inicial
    _game->preRun();

    return true;
}

// -------------------------------------------------------------------------
// loadFiles  (antes GameDriver::loadFiles)
// -------------------------------------------------------------------------

bool Abbey::loadFiles()
{

    for (size_t i = 0; i < _gameFiles.size(); i++) {
        if (!_fileLoader->loadGameData("abadia", _gameFiles[i])) {
            sys->print("Error: no se pueden cargar los ficheros del juego.\n");
            for (size_t j = 0; j <= i; j++)
                _gameFiles[j]->free();
            return false;
        }
    } 
    return true;
}

// -------------------------------------------------------------------------
// filesLoaded  (antes AbadiaDriver::filesLoaded)
// -------------------------------------------------------------------------

void Abbey::filesLoaded()
{ 
    int bytesWritten;
    UINT8 auxBuffer[0xff00];

    const int vgaSize   = _gameFiles[1]->getTotalSize();   // 174065
    const int flipExtra = 21600;

    _romsPtr = new UINT8[0x24000 + (vgaSize + flipExtra) * 3];

    if (!_romsPtr) {
        sys->print("Error: sin memoria para la ROM.\n");
        return;
    }

    DskReader dsk(_gameFiles[0]->getData());

    // Pistas 0x01-0x11
    for (int i = 0x01; i <= 0x11; i++)
        dsk.getTrackData(i, &auxBuffer[(i - 0x01) * 0x0f00], 0x0f00, bytesWritten);

    reOrderAndCopy(&auxBuffer[0x0000], &_romsPtr[0x00000], 0x4000); // abadia0.bin
    reOrderAndCopy(&auxBuffer[0x4000], &_romsPtr[0x0c000], 0x4000); // abadia3.bin
    reOrderAndCopy(&auxBuffer[0x8000], &_romsPtr[0x20000], 0x4000); // abadia8.bin
    reOrderAndCopy(&auxBuffer[0xc000], &_romsPtr[0x04100], 0x3f00); // abadia1.bin

    // Pistas 0x12-0x16
    for (int i = 0x12; i <= 0x16; i++)
        dsk.getTrackData(i, &auxBuffer[(i - 0x12) * 0x0f00], 0x0f00, bytesWritten);
    reOrderAndCopy(&auxBuffer[0x0000], &_romsPtr[0x1c000], 0x4000); // abadia7.bin

    // Pistas 0x17-0x1b
    for (int i = 0x17; i <= 0x1b; i++)
        dsk.getTrackData(i, &auxBuffer[(i - 0x17) * 0x0f00], 0x0f00, bytesWritten);
    reOrderAndCopy(&auxBuffer[0x0000], &_romsPtr[0x18000], 0x4000); // abadia6.bin

    // Pistas 0x1c-0x21
    for (int i = 0x1c; i <= 0x21; i++)
        dsk.getTrackData(i, &auxBuffer[(i - 0x1c) * 0x0f00], 0x0f00, bytesWritten);
    reOrderAndCopy(&auxBuffer[0x0000], &_romsPtr[0x14000], 0x4000); // abadia5.bin

    // Pistas 0x21-0x25
    for (int i = 0x21; i <= 0x25; i++)
        dsk.getTrackData(i, &auxBuffer[(i - 0x21) * 0x0f00], 0x0f00, bytesWritten);
    reOrderAndCopy(&auxBuffer[0x0000], &_romsPtr[0x08000], 0x4000); // abadia2.bin

    // Gráficos VGA y CPC después de la ROM
    const int base = 0x24000 - 1;
    memcpy(&_romsPtr[base],
           _gameFiles[1]->getData(), vgaSize);
    memcpy(&_romsPtr[base + vgaSize + flipExtra],
           _gameFiles[1]->getData(), vgaSize);
    memcpy(&_romsPtr[base + (vgaSize + flipExtra) * 2],
           _gameFiles[2]->getData(), vgaSize);

    // Parche texto COMPLETAS: - y . → # y ~
    UINT8 *tmp = &_romsPtr[0x4000 + 0x4fbc + 7 * 6];
    *(tmp + 3) = '#';
    *(tmp + 4) = '~'; 
}

void Abbey::deallocateFilesMemory()
{
    for (auto *e : _gameFiles)
        e->free(); 
}

// -------------------------------------------------------------------------
// finishInit  (antes AbadiaDriver::finishInit)
// -------------------------------------------------------------------------

void Abbey::finishInit()
{ 
    _game    = new Abadia::Juego(_romsPtr); 
}

// -------------------------------------------------------------------------
// mainLoop  (antes Game::mainLoop)
// -------------------------------------------------------------------------

void Abbey::mainLoop()
{ 
	/*
    while (!sys->exit)
    {
        auto frameTime = SDL_GetTicks();

        handleEvents();
        logic();
	// (antes AbadiaDriver::render + Game::render)
    	sys->updateTexture();
    	sys->updateScreen();   // antes en Game::render  
	// TODO: usar init y endFrame para ver cuando hacer updateTexture
	// y cuando updateScreen

        if (SDL_GetTicks() - frameTime < sys->minimumFrameTime)
            SDL_Delay(sys->minimumFrameTime - (SDL_GetTicks() - frameTime)); 
    }  */
#ifndef __EMSCRIPTEN__
        while(!sys->exit)
        {
		sys->initFrame();

                handleEvents();
                logic();
    		sys->updateTexture();
                sys->updateScreen();

		sys->endFrame();
        }
#else
        if (sys->exit) {
                sys->quit();
                emscripten_cancel_main_loop();
        }
        sys->initFrame();

        if (sys->logicInterrupt) {
                handleEvents();
                logic();
        }
	sys->updateTexture();
	sys->updateScreen();

        sys->endFrame();
#endif
}

// -------------------------------------------------------------------------
// handleEvents  (antes Game::handleEvents)
// -------------------------------------------------------------------------

void Abbey::handleEvents()
{
    sys->handleEvents();

    if (sys->pad.start) {
        _game->changeState(Abadia::STATES::MENU);
        sys->pad.start = false;
    } 
}

// -------------------------------------------------------------------------
// logic  (antes Game::logic + Vigasoco::mainLoop + AbadiaDriver::runSync/runAsync)
// -------------------------------------------------------------------------

void Abbey::logic()
{ 
    if (sys->informationMode) {
        // antes: Vigasoco::toggleInformationMode → AbadiaDriver::showGameLogic
        _game->modoInformacion       = !_game->modoInformacion;
        _game->cambioModoInformacion = true;
        sys->informationMode = false;
    }

    // runSync (AbadiaDriver): procesa frase del marcador si no está en pausa
    SDL_Log("pausa %d\n",_game->pausa);
    if (!_game->pausa)
        Abadia::elGestorFrases->procesaFraseActual();

    // runAsync (AbadiaDriver): máquina de estados principal del juego 
    _game->stateMachine(); 
}

// -------------------------------------------------------------------------
// reOrderAndCopy  (antes AbadiaDriver::reOrderAndCopy)
// -------------------------------------------------------------------------

void Abbey::reOrderAndCopy(const UINT8 *src, UINT8 *dst, int size)
{ 
    for (int i = 0; i < size; i++)
        dst[size - i - 1] = src[i]; 
}
