#include "abbey.h"
#include "DskReader.h"
#include "GestorFrases.h"
#include "system.h"
#include <cstdio>
#include <cstring>

// -------------------------------------------------------------------------
// Constructor / destructor
// -------------------------------------------------------------------------

Abbey::Abbey()
//    : _palette(nullptr)
//    , _cpc6128(nullptr)
 //   , _game(nullptr)
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
	/*
    delete _cpc6128;
    delete _game;
    delete[] _romsPtr;
//    delete _palette;
    delete _fileLoader;
*/
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
	
    //_palette = new SDLPalette();

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
{ /*
    _cpc6128 = new CPC6128();
    _game    = new Abadia::Juego(_romsPtr, _cpc6128); */
    _game    = new Abadia::Juego(_romsPtr); 
}

// -------------------------------------------------------------------------
// mainLoop  (antes Game::mainLoop)
// -------------------------------------------------------------------------

void Abbey::mainLoop()
{ 
    while (!sys->exit)
    {
        auto frameTime = SDL_GetTicks();

        handleEvents();
        logic();
        //renderFrame();
    	sys->updateTexture();
    	sys->updateScreen();   // antes en Game::render  

        if (SDL_GetTicks() - frameTime < sys->minimumFrameTime)
            SDL_Delay(sys->minimumFrameTime - (SDL_GetTicks() - frameTime)); 
    } 
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
    if (!_game->pausa)
        Abadia::elGestorFrases->procesaFraseActual();

    // runAsync (AbadiaDriver): máquina de estados principal del juego 
    _game->stateMachine(); 
}

// -------------------------------------------------------------------------
// renderFrame  (antes AbadiaDriver::render + Game::render)
// -------------------------------------------------------------------------

void Abbey::renderFrame()
{ /*
    auto  *posPant    = _cpc6128->screenBuffer;
    UINT8 *posPantTmp = nullptr;
    auto  *pixels     = static_cast<uint8_t *>(sys->surface->pixels);
    auto   p          = reinterpret_cast<Uint32 *>(pixels);

    for (int y = 0; y < TEXTURE_HEIGHT / 2; y++)
    {
        posPantTmp = posPant;

        // Primera pasada (línea original)
        for (int x = 0; x < TEXTURE_WIDTH; x++) {
            const auto data = *posPant++;
            *p++ = sys->RGBA(
0xFF, //               static_cast<Uint8>(_game->paleta->paleta2->_palette[data].R),
0xFF,//                static_cast<Uint8>(_game->paleta->paleta2->_palette[data].G),
0xFF,//                static_cast<Uint8>(_game->paleta->paleta2->_palette[data].B),
                0xFF);
        }

        posPant = posPantTmp;

        // Segunda pasada (línea duplicada — escalado x2 vertical)
        for (int x = 0; x < TEXTURE_WIDTH; x++) {
            const auto data = *posPant++;
            *p++ = sys->RGBA(
0xFF,//                static_cast<Uint8>(_game->paleta->paleta2->_palette[data].R),
      0xFF,//          static_cast<Uint8>(_game->paleta->paleta2->_palette[data].G),
            0xFF,//    static_cast<Uint8>(_game->paleta->paleta2->_palette[data].B),
                0xFF);
        }
    }
*/
//	sys->fillMode1Rect(0,0,320,200,1); 
//
/*
for (int kk=0;kk<320;kk++)
       for(int zz=0;zz<100;zz++)	sys->setRGBPixel(kk,zz,255);
    sys->updateTexture();
    sys->updateScreen();   // antes en Game::render  
			   */
}

// -------------------------------------------------------------------------
// reOrderAndCopy  (antes AbadiaDriver::reOrderAndCopy)
// -------------------------------------------------------------------------

/*static*/ void Abbey::reOrderAndCopy(const UINT8 *src, UINT8 *dst, int size)
{ 
    for (int i = 0; i < size; i++)
        dst[size - i - 1] = src[i]; 
}
