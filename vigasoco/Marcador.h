// Marcador.h
//
//		Clase que encapsula las acciones relacionadas con el marcador
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _MARCADOR_H_
#define _MARCADOR_H_


#include <string>
#include "Singleton.h"


namespace Abadia {

#define elMarcador Marcador::getSingletonPtr()

class Marcador : public Singleton<Marcador>
{
	// campos
public:
	UINT8 *roms;			// puntero a las roms originales

	int numPosScrollDia;		// número de posiciones para completar el scroll del nombre del día
	UINT8 *nombreMomentoDia;	// apunta al nombre del momento actual del día

protected:
	static int duracionEtapasDia[7][7];

	// métodos
public:
	void dibujaMarcador();
	void limpiaAreaMarcador();
	void limpiaAreaFrases();
	void dibujaObjetos(int objetos, int mascara);

	void muestraDiaYMomentoDia();
	void avanzaMomentoDia();
	void realizaScrollMomentoDia();

	void decrementaObsequium(int unidades);

	void imprimeFrase(std::string frase, int x, int y, int colorTexto, int colorFondo);
	void imprimirCaracter(int caracter, int x, int y, int colorTexto, int colorFondo);

	// inicialización y limpieza
	Marcador();
	~Marcador();

protected:
	void dibujaDia(int numDia);
	void dibujaDigitoDia(int digito, int x, int y);
	void dibujaBarra(int lgtud, int color, int x, int y);
};

}

#endif	// _MARCADOR_H_
