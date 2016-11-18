#include "Jugador.h"
#include "ArmaS.h"
#include "ArmaH.h"
#include "ArmaR.h"
#include <windows.h>

Jugador::Jugador(std::string UnNombre, std::string UnColor)
{
	Nombre = UnNombre;
	Color = UnColor;
	Estado = "QUIETO-DER";
	EstadoAnterior = Estado;
	Direccion = "DERECHA";
	x = 20;
	y = 365;
	puntaje = 0;
	vida = 100;
	Conectado = true;
	Saltando = false;
	SaltandoVertical = false;

	NumeroArma = 0;
	Armas[0] = new ArmaS();
	Armas[1] = new ArmaH();
	Armas[2] = new ArmaR();
}

void Jugador::ArmaSiguiente() {
	NumeroArma++;

	if (NumeroArma > 2) {
		NumeroArma = 0;
	}
}

void Jugador::ArmaAnterior() {
	NumeroArma--;

	if (NumeroArma < 0) {
		NumeroArma = 2;
	}
}
std::string Jugador::GetDireccion() {

	return Direccion;
}

int Jugador::getPuntaje()
{
	return puntaje;
}

int Jugador::GetVida()
{
	return vida;
}

Arma* Jugador::GetArmaEnUso() {

	return Armas[NumeroArma];
}

void Jugador::herirEnemigo()
{
	Arma* UnArma = GetArmaEnUso();

	if(UnArma->GetCodigoArma() == "S")
	{
		puntaje+=35;
	}
	if(UnArma->GetCodigoArma() == "H")
	{
		puntaje+=15;
	}
	if(UnArma->GetCodigoArma() == "R")
	{
		puntaje+=40;
	}
}
void Jugador::Mover(std::string Tecla) {

	if (Tecla == "SPACE") {

		SaltandoVertical = true;

		if (!EstaSaltando()) {

			TiempoInicioSaltoY = clock();
			PosicionYInicioSalto = y;

			EstadoAnterior = this->Estado;

			if (EstaCaminando()) {
				if (this->EstaApuntandoALaDerecha()) {
					Tecla = "RIGHT";
				}
				else {
					Tecla = "LEFT";
				}
			}

			SetEstaSaltando(true);
		}
	}

	if (Tecla == "RIGHT")  {
		
		if (!EstaSaltando()) {

			this->Estado = "CAMINA-DER";
		}
		else {

			if (!((!SaltandoVertical) && (this->Estado == "SALTANDO-DER"))) {

				SaltandoVertical = false;
				this->Estado = "SALTANDO-DER";
				SetEstadoAnterior("CAMINA-DER");
				TiempoInicioSaltoX = clock();
				PosicionXInicioSalto = x;
			}
		}

		if ((this->EstaApuntandoArriba()) && (this->GetArmaEnUso()->PuedeApuntarEnDiagonal())) {
			Direccion = "ARRIBA-DER-DIAG";
			this->Estado = "DIAGONAL-DER";
		}
		else {
			Direccion = "DERECHA";
		}
	}

	if (Tecla == "LEFT") {

		if (!EstaSaltando()) {

			this->Estado = "CAMINA-IZQ";
		}  
		else {
			if (!((!SaltandoVertical) && (this->Estado == "SALTANDO-IZQ"))) {

				SaltandoVertical = false;
				this->Estado = "SALTANDO-IZQ";
				SetEstadoAnterior("CAMINA-IZQ");
				TiempoInicioSaltoX = clock();
				PosicionXInicioSalto = x;
			}
		}

		if ((this->EstaApuntandoArriba()) && (this->GetArmaEnUso()->PuedeApuntarEnDiagonal())) {
			Direccion = "ARRIBA-IZQ-DIAG";
			this->Estado = "DIAGONAL-IZQ";
		} else {
			Direccion = "IZQUIERDA";
		}
	}

	if (Tecla == "UP") {

		bool EstabaCaminando = false;

		if (EstaSaltando()) {
			SaltandoVertical = true;
		}

		if ((this->EstaCaminando()) && (this->GetArmaEnUso()->PuedeApuntarEnDiagonal())) {
			EstabaCaminando = true;
			this->SetEstadoAnterior(this->GetEstado());
			if (this->EstaApuntandoALaDerecha()) {
				Direccion = "ARRIBA-DER-DIAG";
				this->Estado = "DIAGONAL-DER";
			} else {
				Direccion = "ARRIBA-IZQ-DIAG";
				this->Estado = "DIAGONAL-IZQ";
			}
		}
		else {
			if (this->EstaApuntandoALaDerecha()) {
				Direccion = "ARRIBA-DER";
				this->Estado = "ARRIBA-DER";
			}
			else {
				Direccion = "ARRIBA-IZQ";
				this->Estado = "ARRIBA-IZQ";
			}
		}
	}

	if (Tecla == "DOWN") {
		
		if (EstaSaltando()) {
			SaltandoVertical = true;
		}
	}
}

bool Jugador::EstaApuntandoAbajo() {

	return ((Direccion == "ABAJO") || (Direccion == "ABAJO-IZQ") || (Direccion == "ABAJO-DER"));
}

bool Jugador::EstaApuntandoArriba() {

	return ((Direccion == "ARRIBA") || (Direccion == "ARRIBA-IZQ") || (Direccion == "ARRIBA-DER"));
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

	return ((this->Estado == "QUIETO-DER-DISPARA") || (this->Estado == "QUIETO-IZQ-DISPARA") ||
			(this->Estado == "ARRIBA-DER-DISPARA") || (this->Estado == "ARRIBA-IZQ-DISPARA") ||
			(this->Estado == "DIAGONAL-DER-DISPARA") || (this->Estado == "DIAGONAL-IZQ-DISPARA"));
}

void Jugador::SetEstaSaltando(bool UnEstaSaltando) {

	Saltando = UnEstaSaltando;

	if (Saltando) {
		if (EstaApuntandoALaDerecha()) {

			this->Estado = "SALTANDO-DER";
		}
		else {
			this->Estado = "SALTANDO-IZQ";		
		}
	}
	else {
		this->Estado = this->GetEstadoAnterior();
	}
}

void Jugador::SetEstaSaltandoVertical(bool UnEstaSaltandoVertical) {

	SaltandoVertical = UnEstaSaltandoVertical;
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

	return "Player" + Color + "-" + GetArmaEnUso()->GetCodigoArma();
}

std::string Jugador::GetEstado() {


	// Este es un workaround pedorro -> Habria que ver cuando y porque el estado se pone vacio
	if (Estado == "") {
		Estado = "QUIETO-DER";
	}

	return Estado;
}

int Jugador::GetX() {
	
	return x;
}

int Jugador::GetY() {

	return y;
}

bool Jugador::EstaSaltando() {

	return Saltando;
}

bool Jugador::EstaSaltandoVertical() {

	return SaltandoVertical;
}

void Jugador::SueltaTeclaDireccion() {
	if (this->EstaCaminando()) {
		if (this->EstaApuntandoALaDerecha()) {

			this->SetDireccion("DERECHA");
			this->SetEstado("CAMINA-DER");
		}
		else {

			this->SetDireccion("IZQUIERDA");
			this->SetEstado("CAMINA-IZQ");
		}
	}
	else {
		if (this->EstaApuntandoALaDerecha()) {

			this->SetDireccion("DERECHA");
			this->SetEstado("QUIETO-DER");
		}
		else {

			this->SetDireccion("IZQUIERDA");
			this->SetEstado("QUIETO-IZQ");
		}
		// TODO: if EstaSaltando then setEstadoANTERIOR
	}
}

bool Jugador::EstaCaminando() {

	return ((this->Estado == "CAMINA-DER") || (this->Estado == "CAMINA-IZQ"));
}

bool Jugador::EstaApuntandoALaDerecha() {

	return ((this->Direccion == "DERECHA") || (this->Direccion == "ARRIBA-DER") ||
			(this->Direccion == "ARRIBA-DER-DIAG") || (this->Direccion == "ABAJO-DER") ||
			(this->Direccion == "ABAJO-DER-DIAG"));
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

void Jugador::SetDireccion(std::string UnaDireccion) {

	Direccion = UnaDireccion;
}

void Jugador::SetY(int UnY) {

	y = UnY;
}

Jugador::~Jugador()
{
}
