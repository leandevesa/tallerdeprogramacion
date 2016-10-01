#pragma once
#include <string>
class Jugador
{
public:
	Jugador(std::string UnNombre, std::string UnIDSprite);
	~Jugador();
	void Mover(std::string Direccion);
	std::string GetNombre();
	std::string GetIDSprite();
	std::string GetEstado();
	int GetX();
	int GetY();
private:
	std::string Nombre;
	std::string IDSprite;
	std::string Estado;
	int x;
	int y;
};
