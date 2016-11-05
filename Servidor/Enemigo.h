#pragma once
#include <string>
class Enemigo
{
public:
	Enemigo(std::string unIdSprite, int posX, int posY, int vel, int unaVida);
	int getX();
	int getY();
	void mover();
	void disparar();
	std::string getEstado();
	std::string getID();
	int getVelocidad();
	~Enemigo(void);
private:
	std::string IDSprite;
	std::string estado;
	int vida;
	int velocidad;
	int x;
	int y;
};

