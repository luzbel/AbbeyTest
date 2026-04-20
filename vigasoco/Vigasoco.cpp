// Vigasoco.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "Vigasoco.h"
#include "AbadiaDriver.h"

/////////////////////////////////////////////////////////////////////////////
// initialization and cleanup
/////////////////////////////////////////////////////////////////////////////

Vigasoco::Vigasoco()
{
	_driver = 0;
}

Vigasoco::~Vigasoco()
{
}

/////////////////////////////////////////////////////////////////////////////
// init
/////////////////////////////////////////////////////////////////////////////
bool Vigasoco::init()
{
	
	// creates the game driver
	_driver = createGameDriver("abadia");

	if (!_driver) return false;

	// inits the game driver (load files, decode gfx, preprocessing, etc)
	if (!_driver->init()) return false;	

	// calls template method to perform specific actions after initialization has been completed
	_driver->preRun();

	return true;    
}

void Vigasoco::end()
{
}

void Vigasoco::mainLoop()
{
	_driver->runAsync();
	_driver->runSync();
	_driver->render();
}

GameDriver * Vigasoco::createGameDriver(std::string game)
{
	// TODO: move this to a factory	
	return new AbadiaDriver();	
}

void Vigasoco::toggleInformationMode()
{
	_driver->showGameLogic();
}

void Vigasoco::showMenu()
{
	_driver->showMenu();
}

void Vigasoco::changeState(Abadia::STATES newState)
{
	_driver->changeState(newState);
}
