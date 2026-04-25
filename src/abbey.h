#ifndef ABBEY_H
#define ABBEY_H

#include <string>
#include "system.h"
#include "SDLPaleta.h"
//#include "cpc6128.h"
#include "Juego.h"
#include "FileLoader.h"
#include "GameDataEntity.h"

// Abbey reemplaza las capas Game + Vigasoco + GameDriver + AbadiaDriver.
// Responsabilidad única: inicializar el juego, ejecutar el bucle principal
// y limpiar recursos. La lógica del juego sigue en Juego.

class Abbey
{
public:
    Abbey();
    ~Abbey();

    // Devuelve false si la inicialización falla (ficheros no encontrados, etc.)
    bool init();

    // Bucle principal; retorna cuando sys->exit es true
    void mainLoop();

private:
    // --- sub-pasos de init() ---
    bool loadFiles();
    void filesLoaded();           // extracción DSK + memcpy VGA/CPC + parche ROM
    void deallocateFilesMemory();
    void finishInit();            // new CPC6128 + new Juego

    // --- sub-pasos de mainLoop() ---
    void handleEvents();
    void logic();
    void renderFrame();           // loop VGA→SDL + updateTexture + updateScreen

    // --- helpers ---
    static void reOrderAndCopy(const UINT8 *src, UINT8 *dst, int size);

    // --- recursos propios ---
//    SDLPalette             *_palette;
//    CPC6128                *_cpc6128;
    Abadia::Juego          *_game;
    FileLoader             *_fileLoader;

    // GameDataEntities (dsk, GraficosVGA, GraficosCPC)
    std::vector<GameDataEntity *> _gameFiles;

    UINT8 *_romsPtr;   // bloque de memoria con ROM + gráficos
};

#endif // ABBEY_H
