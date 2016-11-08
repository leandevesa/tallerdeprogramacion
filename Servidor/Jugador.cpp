#include "Jugador.h"
#include <windows.h>

Jugador::Jugador(std::string UnNombre, std::string UnIDSprite)
{
	Nombre = UnNombre;
	IDSprite = UnIDSprite;
	Estado = "QUIETO-DER";
	x = 20;
	y = 405;
	vida = 100;
	Conectado = true;
	UnArma = new Arma("Arma_H", "DERECHA", 200);
}

void Jugador::Mover(std::string Tecla) {

	if (Tecla == "SPACE") {

		if (!EstaSaltando()) {

			TiempoInicioSaltoY = clock();
			PosicionYInicioSalto = y;

			if (EstaCaminando()) {

				EstadoAnterior = this->Estado;
				if (this->EstaApuntandoALaDerecha()) {
					Tecla = "RIGHT";
				} else {
					Tecla = "LEFT";
				}
			}

			this->Estado = "SALTANDO";  // TODO: Fix salta y cambia de direccion
		}
		else {
			if ((this->Estado != "SALTANDO-IZQ") || (this->Estado != "SALTANDO-DER")) {

				this->Estado = "SALTANDO";
			}
		}

	}

	if (Tecla == "RIGHT")  {
		
		if (!EstaSaltando()) {

			this->Estado = "CAMINA-DER";
		}
		else {
			if (this->Estado != "SALTANDO-DER") {

				this->Estado = "SALTANDO-DER";
				SetEstadoAnterior("CAMINA-DER");
				TiempoInicioSaltoX = clock();
				PosicionXInicioSalto = x;
			}
		}

		UnArma->CambiarDireccion("DERECHA");
	}

	if (Tecla == "LEFT") {

		if (!EstaSaltando()) {

			this->Estado = "CAMINA-IZQ";
		}  
		else {
			if (this->Estado != "SALTANDO-IZQ") {

				this->Estado = "SALTANDO-IZQ";
				SetEstadoAnterior("CAMINA-IZQ");
				TiempoInicioSaltoX = clock();
				PosicionXInicioSalto = x;
			}
		}

		UnArma->CambiarDireccion("IZQUIERDA");
	}

	if (Tecla == "DOWN") {

		UnArma->CambiarDireccion("ABAJO");
	}
}
Arma* Jugador::GetArma() 
{

	return UnArma;
}
void Jugador::SetEstaConectado(bool EstaConectado)
{
	this->Conectado = EstaConectado;

	if(!EstaConectado)
	{
		this->Estado = "DESCONECTADO";
		this->x = 20;
		this->y = 405;
	}
	else {
		this->Estado = "QUIETO-DER";
	}
}

bool Jugador::EstaDisparando() {

	return ((this->Estado == "DISPARA-DER") || (this->Estado == "DISPARA-IZQ"));
}

bool Jugador::GetEstaConectado() {

	return Conectado;
}

void Jugador::MoverEnX(int UnX) {

	x += UnX;
}

void Jugador::SetEstado(std::string nuevoEstado) {

	this->Estado = nuevoEstado;
}

void Jugador::SetEstadoAnterior(std::string nuevoEstado) {

	this->EstadoAnterior = nuevoEstado;
}

std::string Jugador::GetNombre() {

	return Nombre;
}

std::string Jugador::GetIDSprite() {

	return IDSprite;
}

std::string Jugador::GetEstado() {

	return Estado;
}

int Jugador::GetX() {
	
	return x;
}

int Jugador::GetY() {

	return y;
}

bool Jugador::EstaSaltando() {

	return ((this->Estado == "SALTANDO") || (this->Estado == "SALTANDO-DER") || (this->Estado == "SALTANDO-IZQ"));
}

bool Jugador::EstaCaminando() {

	return ((this->Estado == "CAMINA-DER") || (this->Estado == "CAMINA-IZQ"));
}

bool Jugador::EstaApuntandoALaDerecha() {

	return ((this->Estado == "CAMINA-DER") || (this->Estado == "SALTANDO-DER") ||
			(this->Estado == "QUIETO-DER"));
}

float Jugador::GetTiempoInicioSaltoY() {

	return TiempoInicioSaltoY;
}
float Jugador::GetTiempoInicioSaltoX() {

	return TiempoInicioSaltoX;
}

int Jugador::GetPosicionYInicioSalto() {

	return PosicionYInicioSalto;
}

int Jugador::GetPosicionXInicioSalto() {

	return PosicionXInicioSalto;
}

std::string Jugador::GetEstadoAnterior() {

	return EstadoAnterior;
}

void Jugador::SetX(int UnX) {

	x = UnX;
}

void Jugador::SetY(int UnY) {

	y = UnY;
}

Jugador::~Jugador()
{
}
