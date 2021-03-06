#pragma once
#include <string>
#include <ctime>
#include "Arma.h"
class Jugador
{
public:
	Jugador(std::string UnNombre, std::string UnColor, int Width, int Height, bool UnEsDios);
	~Jugador();
	void Mover(std::string Direccion);
	std::string GetNombre();
	std::string GetIDSprite();
	std::string GetEstado();
	std::string GetDireccion();
	void SetEstado(std::string nuevoEstado);
	void SetEstadoAnterior(std::string nuevoEstado);
	void SetEstaConectado(bool EstaConectado);
	bool GetEstaConectado();
	void SetX(int UnX);
	void SetY(int UnY);
	std::string GetEstadoAnterior();
	bool EstaApuntandoALaDerecha();
	bool EstaApuntandoAbajo();
	bool EstaApuntandoArriba();
	float GetTiempoInicioSaltoY();
	float GetTiempoInicioSaltoX();
	int GetPosicionYInicioSalto();
	int GetPosicionXInicioSalto();
	int GetX();
	int GetY();
	int GetWidth();
	int GetHeight();
	void MoverEnX(int UnX);
	bool EstaCaminando();
	bool EstaSaltando();
	bool EstaSaltandoVertical();
	bool EstaDisparando();
	void herirEnemigo();
	int getPuntaje();
	int GetVida();
	void SetEstaSaltando(bool UnEstaSaltando);
	void SetEstaSaltandoVertical(bool UnEstaSaltandoVertical);
	void SetDireccion(std::string UnaDireccion);
	void SueltaTeclaDireccion();
	void ArmaSiguiente();
	void ArmaAnterior();
	void reestablecerVida();
	bool EstaCayendo();
	Arma* GetArmaEnUso();
	void SetParametrosSalto(int UnTiempoInicioSaltoX, int UnTiempoInicioSaltoY, int UnaPosicionXInicioSalto, int PosicionYInicioSalto);
	float GetVelocidadX();
	float GetVelocidadY();
	void SetVelocidadX(float UnaVelocidad);
	void SetVelocidadY(float UnaVelocidad);
	void UpdatePos();
	void sumarBalas();
	void SetEstaEnPiso(bool UnEstaEnPiso);
	void SacarVida(int Cantidad);
	void muereJugador();
private:
	bool EsDios;
	bool EstaEnPiso;
	bool Saltando;
	bool SaltandoVertical;
	std::string Direccion;
	std::string Nombre;
	std::string Color;
	std::string Estado;
	std::string EstadoAnterior;
	float TiempoInicioSaltoY;
	float TiempoInicioSaltoX;
	int PosicionYInicioSalto;
	int PosicionXInicioSalto;
	int x;
	int y;
	float VelocidadX;
	float VelocidadY;
	int Width;
	int Height;
	int puntaje;
	int vida;
	bool Conectado;
	Arma* Armas[3];
	int NumeroArma;
};

