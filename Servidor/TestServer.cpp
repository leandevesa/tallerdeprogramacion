#undef UNICODE
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <process.h>

#include "Server.h"
#include "Juego.h"
#include "Usuarios.h"
#include "Mensaje.h"
#include "Globales.h"
#include "tinyxml2.h"

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#ifdef WIN32 
#define ClearScreen() system("cls");
#define pause() system("pause");
#else 
#define ClearScreen() system("clear");
#define pause() system("pause");
#endif

Server UnServer;
Usuarios ControlUsuarios;
#define MAX_CLIENTES 6
int CantidadClientes = 0;

int Nivel = 1;

string ArchivoEscenarios = "";

HANDLE ghMutex;
Juego UnJuego;

void MostrarListaComandos() {
	cout << "Ingrese la letra ""q"" si desea apagar el servidor: ";
}

void PedirModoDeJuego()
{
	std::string opcion = "0";
	while(opcion != "1" && opcion != "2" && opcion != "3")
	{
		cout << endl << "1- Individual MP 2-Colaborativo MP 3-Grupal MP" << endl;
		std::getline(std::cin, opcion);
	}
	UnJuego.establecerModo(opcion);
}

std::string ObtenerTextoPuntaje(int UnModo, int IndiceMiJugador) {
	std::string TextoPuntaje;

	Jugador* MiJugador;
	int CantJugadores = UnJuego.GetCantJugadores();
	//TextoPuntaje = "Puntaje";
	int Puntaje = 0;
	MiJugador = UnJuego.GetJugador(IndiceMiJugador);
	TextoPuntaje = MiJugador->GetNombre();
	Puntaje = MiJugador->getPuntaje();
	/* Lean, te comento esto porque quizas pueda servir, pero me parece que los
	puntajes, a lo largo de cada nivel, se muestran por jugador independientemente del modo.
	switch (UnModo) {

		case 1:

			MiJugador = UnJuego.GetJugador(IndiceMiJugador);
			TextoPuntaje = MiJugador->GetNombre();
			Puntaje = MiJugador->getPuntaje();
			break;
		case 2:

			TextoPuntaje = TextoPuntaje + "(TODOS)";

			for (int i = 0; i < CantJugadores; i++) {

				Jugador* OtroJugador = UnJuego.GetJugador(i);
				
				Puntaje += OtroJugador->getPuntaje();
			}
			break;
		case 3:

			MiJugador = UnJuego.GetJugador(IndiceMiJugador);
			Equipo *UnEquipo = UnJuego.GetEquipoJugador(MiJugador->GetNombre());

			TextoPuntaje = TextoPuntaje + "(" + UnEquipo->GetNombresJugadores() + ")";
			Puntaje = UnEquipo->obtenerPuntaje();
			break;
	}
	*/
	TextoPuntaje = TextoPuntaje + ": " + IntAString(Puntaje);	

	return TextoPuntaje;
}

void CargarSprites() {

	Lista<DatosSprites *>* ListaSprites = new Lista<DatosSprites *>();

	tinyxml2::XMLDocument docu;

	std::string path = "Archivos\\escenariodef" + IntAString(Nivel) + ".xml";

	char* pathXML = strdup(path.c_str());

	if (docu.LoadFile(pathXML) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{

		tinyxml2::XMLElement* elementoEscenario = docu.FirstChildElement();

		tinyxml2::XMLElement* elementoSprites = elementoEscenario->FirstChildElement("SPRITES");

		const char* cantidadSrites = elementoSprites->Attribute("cantidad");
		//iterando sobre todos los Sprites 
		int contador = 0;
		for (tinyxml2::XMLElement* elementoSprite = elementoSprites->FirstChildElement("SPRITE"); elementoSprite != NULL; elementoSprite = elementoSprite->NextSiblingElement("SPRITE"))
		{

			// Envio info de los sprites a cargar
			const char* idSprite = elementoSprite->Attribute("id");

			const char* frameWidthSprite = elementoSprite->Attribute("frameWidth");

			const char* frameHeightSprite = elementoSprite->Attribute("frameHeight");

			const char* velocidadSprite = elementoSprite->Attribute("velocidad");

			DatosSprites* UnSprite = new DatosSprites();
			UnSprite->id = idSprite;
			UnSprite->width = stoi(frameWidthSprite);
			UnSprite->height = stoi(frameHeightSprite);
			UnSprite->velocidad = stoi(velocidadSprite);
			ListaSprites->agregar(UnSprite);

			tinyxml2::XMLElement* elementoEstados = elementoSprite->FirstChildElement("ESTADOS");
			const char* cantidadEstados = elementoEstados->Attribute("cantidad");

																							//iterando sobre todos los estados de un Sprite
			for (tinyxml2::XMLElement* elementoEstado = elementoEstados->FirstChildElement("ESTADO"); elementoEstado != NULL; elementoEstado = elementoEstado->NextSiblingElement("ESTADO"))
			{

				const char* idEstado = elementoEstado->Attribute("id");
				const char* cantFramesEstado = elementoEstado->Attribute("cantFrames");
			}

		}

	}

	UnJuego.SetListaDatosSprites(ListaSprites);
}

void CargarPlataformas() {
	tinyxml2::XMLDocument docu;

	UnJuego.BorrarPlataformas();

	std::string path = "Archivos\\escenariodef" + IntAString(Nivel) + ".xml";

	char* pathXML = strdup(path.c_str());

	if (docu.LoadFile(pathXML) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{
		
		tinyxml2::XMLElement* Plataformas = docu.FirstChildElement()->FirstChildElement("PLATAFORMAS");

		for (tinyxml2::XMLElement* Plataforma = Plataformas->FirstChildElement("PLATAFORMA");
			Plataforma != NULL; Plataforma = Plataforma->NextSiblingElement("PLATAFORMA"))
		{
			const char* X = Plataforma->Attribute("x");
			const char* Ancho = Plataforma->Attribute("ancho");
			const char* Y = Plataforma->Attribute("y");
			const char* Alto = Plataforma->Attribute("alto");

			UnJuego.AgregarPlataforma(stoi(X), stoi(Y), stoi(Ancho), stoi(Alto));
		}
	}
}

void CargarEnemigos() {
	tinyxml2::XMLDocument docu;
	UnJuego.cargarEnemigos();
	//solo se cargan para el primer nivel

	UnJuego.BorrarEnemigos();

	std::string path = "Archivos\\escenariodef" + IntAString(Nivel) + ".xml";

	char* pathXML = strdup(path.c_str());

	if (docu.LoadFile(pathXML) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{

		//Cargo enemigos de cada nivel
		tinyxml2::XMLElement* elementoEscenario = docu.FirstChildElement();
		tinyxml2::XMLElement* elementosNiveles = elementoEscenario->FirstChildElement("NIVELES");
		tinyxml2::XMLElement* elementosNivel = elementosNiveles->FirstChildElement("NIVEL");
		//Nivel 1
		const char* idNivel = elementosNivel->Attribute("id");

		tinyxml2::XMLElement* elementoEnemigos = elementosNivel->FirstChildElement("ENEMIGOS");
		const char* cantidadEnemigos = elementoEnemigos->Attribute("cantidad");

		int nrEnemigo = 1;
		int cantTotalEnemigos = stoi(cantidadEnemigos);

		for (tinyxml2::XMLElement* elementoEnemigo = elementoEnemigos->FirstChildElement("ENEMIGO"); elementoEnemigo != NULL; elementoEnemigo = elementoEnemigo->NextSiblingElement("ENEMIGO"))
		{
			const char* tipo = elementoEnemigo->Attribute("tipo");

			const char* direccion = elementoEnemigo->Attribute("direccion");

			const char* posX = elementoEnemigo->Attribute("posX");

			const char* posY = elementoEnemigo->Attribute("posY");

			const char* vida = elementoEnemigo->Attribute("vida");

			const char* velocidadCaminata = elementoEnemigo->Attribute("velocidadCaminata");

			// El ultimo enemigo del nivel es considerado como el enemigo final

			bool EsEnemigoFinal = false;

			if (nrEnemigo == cantTotalEnemigos) {

				EsEnemigoFinal = true;
			}

			DatosSprites* UnSprite = UnJuego.BuscarSpriteEnLista(tipo);

			UnJuego.AgregarEnemigo(tipo, stoi(posX), stoi(posY), stoi(velocidadCaminata),
				stoi(vida), EsEnemigoFinal, UnSprite->width * 2, UnSprite->height * 2, direccion);

			nrEnemigo++;
		}
		UnJuego.definirAparicionBonusPower();
		UnJuego.definirAparicionBonusKilAll();
		//carga de respuestos de balas

		tinyxml2::XMLElement* elementoRepuestos = elementoEnemigos->NextSiblingElement("REPUESTOS");
		for (tinyxml2::XMLElement* elementoRepuesto = elementoRepuestos->FirstChildElement("REPUESTO"); elementoRepuesto != NULL; elementoRepuesto = elementoRepuesto->NextSiblingElement("REPUESTO"))
		{
			const char* posX = elementoRepuesto->Attribute("posX");
			const char* posY = elementoRepuesto->Attribute("posY");
			UnJuego.agregarRepuestoArma(stoi(posX), stoi(posY));
		}
	}

	Lista<Enemigo *>* todosLosEnemigos = UnJuego.GetTodosLosEnemigos();
	todosLosEnemigos->iniciarCursor();
	Lista<Enemigo*>* enemigosVivos = UnJuego.GetEnemigosPantalla();
	Lista<int>* posiciones = new Lista<int>();
	int indiceAEliminar = 1;
	while (todosLosEnemigos->avanzarCursor())
	{
		UnJuego.MutexearListaCamaras();
		if (todosLosEnemigos->obtenerCursor()->getX() <= (800 + UnJuego.GetCamara(0)->X))
		{
			enemigosVivos->agregar(todosLosEnemigos->obtenerCursor());
			todosLosEnemigos->obtenerCursor()->setIndexEnListaOriginal(UnJuego.obtenerCantEnemigosAparecidos());
			posiciones->agregar(indiceAEliminar);
			UnJuego.sumarEnemigo();
		}
		UnJuego.DesmutexearListaCamaras();
		indiceAEliminar++;
	}
	posiciones->iniciarCursor();
	int indiceCantEliminados = 0;
	while (posiciones->avanzarCursor())
	{
		todosLosEnemigos->remover(posiciones->obtenerCursor() - indiceCantEliminados);
		indiceCantEliminados++;
	}
}

Lista<std::string>* GetTextosFinNivel(int UnModo) {

	Lista<std::string>* Textos = new Lista<std::string>();
	int CantJugadores = UnJuego.GetCantJugadores();

	if (UnModo == 1) {
		for (int i = 0; i < CantJugadores; i++) {

			Jugador* UnJugador = UnJuego.GetJugador(i);
			Textos->agregar(UnJugador->GetNombre() + ": " + IntAString(UnJugador->getPuntaje()));
		}
	}

	if (UnModo == 2) {
		int Total = 0;

		for (int i = 0; i < CantJugadores; i++) {

			Jugador* UnJugador = UnJuego.GetJugador(i);
			Total += UnJugador->getPuntaje();
		}

		Textos->agregar("Todos: " + IntAString(Total));
	}

	if (UnModo == 3) {
		
		Equipo* UnEquipo;

		UnEquipo = UnJuego.GetEquipo(0);
		Textos->agregar("Equipo 1:");
		Textos->agregar(UnEquipo->GetNombresJugadores() + ": " + IntAString(UnEquipo->obtenerPuntaje()));

		UnEquipo = UnJuego.GetEquipo(1);
		Textos->agregar("Equipo 2:");
		Textos->agregar(UnEquipo->GetNombresJugadores() + ": " + IntAString(UnEquipo->obtenerPuntaje()));
	}

	return Textos;
}

void CargarEscenariosEnJuego() {
	tinyxml2::XMLDocument docu;
	UnJuego.BorrarCamaras();

	std::string path = "Archivos\\escenariodef" + IntAString(Nivel) + ".xml";
	char* pathXML = strdup(path.c_str());

	if (docu.LoadFile(pathXML) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
	{

		tinyxml2::XMLElement* elementoEscenario = docu.FirstChildElement();

		tinyxml2::XMLElement* elementoFondo = elementoEscenario->FirstChildElement("FONDO");

		//iterando sobre todas las capas del fondo 
		for (tinyxml2::XMLElement* elementoCapas = elementoFondo->FirstChildElement("CAPAS"); elementoCapas != NULL; elementoCapas = elementoCapas->NextSiblingElement("CAPAS"))
		{
			for (tinyxml2::XMLElement* elementoImagen = elementoCapas->FirstChildElement("IMAGEN"); elementoImagen != NULL; elementoImagen = elementoImagen->NextSiblingElement("IMAGEN"))
			{
				const char* ancho = elementoImagen->Attribute("width");
				UnJuego.AgregarCamara(stoi(ancho));
			}
		}
	}
}

void MainListenThread(void* arg) {
	string Usuario = "";
	string mensaje = "";
	SOCKET ClientSocket = *(SOCKET*)arg;
	Lista<DatosSprites *>* ListaSprites = new Lista<DatosSprites *>();
	bool EsThreadDePing = false;

	while ((mensaje != "EXIT") && (mensaje != "LOST"))
	{
		mensaje = UnServer.RecibirMensaje(ClientSocket, 4);

		if (mensaje == "AUTH") {

			string UsuarioMsj = UnServer.RecibirMensaje(ClientSocket, 15);
			string PasswordMsj = UnServer.RecibirMensaje(ClientSocket, 15);

			string CodigoRespuesta;
			string MensajeRespuesta;

			if (ControlUsuarios.ContrasenaValida(UsuarioMsj, PasswordMsj)) {

				if (!UnJuego.UsuarioYaLogueado(UsuarioMsj)) {

					Usuario = ToLowerCase(UsuarioMsj);

					Lista<std::string>* UsuariosOnline = UnJuego.GetNombresJugadoresOnline();
					std::string Mensaje = Usuario + " se conecto";
					UnServer.enviarATodos(Mensaje, Usuario, UsuariosOnline);

					std::string Color;

					int cantJugadores = UnJuego.GetCantJugadores();

					switch (cantJugadores)
					{
						case 0:
							Color = "Red";
							break;
						case 1:
							Color = "Yellow";
							break;
						case 2:
							Color = "Violet";
							break;
						case 3: 
							Color = "Blue";
							break;
						default:
							Color = "Blue";
					}

					bool EsDios = ControlUsuarios.EsDios(UsuarioMsj);

					UnJuego.AgregarJugador(Usuario, Color, EsDios);

					CodigoRespuesta = "000";
					MensajeRespuesta = "Player: " + Usuario;

					UnServer.EscribirLog("Usuario " + Usuario + " logueado correctamente.", true);
				} else {

					CodigoRespuesta = "410";
					MensajeRespuesta = UsuarioMsj + " ya se encuentra en logueado.";

					UnServer.EscribirLog("Usuario " + Usuario + " logueado correctamente.", true);
				}
			}
			else {

				CodigoRespuesta = "401";
				MensajeRespuesta = "El usuario y la contrasena no coinciden";

				UnServer.EscribirLog("Fallo de autenticacion de usuario: " + Usuario, true);
			}

			UnServer.EnviarMensaje(CodigoRespuesta, 3, ClientSocket);
			UnServer.EnviarMensaje(MensajeRespuesta, 40, ClientSocket);
		}
		if (mensaje == "EVEN") {

			string Tipo = UnServer.RecibirMensajeTamanoVariable(ClientSocket);
			
			if (Tipo == "RECARGA") {

				if (UnJuego.GetEnemigoFinalMurio()) {

					Nivel++;
					CargarPlataformas();
					CargarEscenariosEnJuego();
					CargarSprites();
					CargarEnemigos();
				}
			}

			UnJuego.RecibirEvento(Usuario, Tipo);
		}
		if(mensaje=="MODO")
		{
			string modo = UnServer.RecibirMensaje(ClientSocket,1);
			UnJuego.establecerModo(modo);
		}
		if (mensaje=="MODE")
		{
			if(UnJuego.obtenerModo() == 0)
			{
				UnServer.EnviarMensaje("NO",2,ClientSocket);
			}
			else
			{
				UnServer.EnviarMensaje("SI",2,ClientSocket);
			}
		}
		if (mensaje=="LIST")
		{
			tinyxml2::XMLDocument docu;
			char* pathXML = strdup(ArchivoEscenarios.c_str());
			docu.LoadFile(pathXML);
			tinyxml2::XMLElement* elementoEscenario = docu.FirstChildElement();
			const char* cantMaxJugadores = elementoEscenario->Attribute("maxJugadores");
			if(UnJuego.GetCantJugadores() != stoi(cantMaxJugadores))
			{
				UnServer.EnviarMensajeTamanoVariable("NOLISTO",ClientSocket);
			}
			else
			{
				UnServer.EnviarMensajeTamanoVariable("OK",ClientSocket);
				UnJuego.EmpezarElJuego();
			}
		}
		if (mensaje == "STAT") {

			std::string GranMensaje = "";

			int CantCamaras = UnJuego.GetCantCamaras();
			GranMensaje.append(IntAString(CantCamaras));
			GranMensaje.append(";");
			for (int i = 0; i < CantCamaras; i++) {
				std::string CamaraX = IntAString(UnJuego.GetCamara(i)->X);
				GranMensaje.append(CamaraX);
				GranMensaje.append(";");
			}
			
			int CantJugadores = UnJuego.GetCantJugadores();
			std::string StrCantJugadores = IntAString(CantJugadores);

			GranMensaje.append(StrCantJugadores);
			GranMensaje.append(";");

			
			// Si hay mensajes para el usuario -> le envio
			Lista<Mensaje*>* Buzon = UnServer.obtenerMensajesPara(Usuario);
			int CantidadMensajes = Buzon->getTamanio();

			GranMensaje.append(IntAString(CantidadMensajes));

			GranMensaje.append(";");
			
			//Envio informacion de los enemigos
			//Cantidad de enemigos que aparecen
			//enviar bonus power
			if (UnJuego.obtenerBonusPower() != 0 && UnJuego.obtenerBonusPower()->getEstado())
			{
				GranMensaje.append("SBP"); //Si Bonus Power
				GranMensaje.append(";");
				GranMensaje.append(UnJuego.obtenerBonusPower()->getInicial());
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.obtenerBonusPower()->getX()));
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.obtenerBonusPower()->getY()));
				GranMensaje.append(";");
				//UnJuego.bonusYaMostrado();
			}
			else
			{
				GranMensaje.append("NB"); //No Bonus
				GranMensaje.append(";");
			}
			//envio bonus kill all
			if (UnJuego.obtenerBonusKillAll() != 0 && UnJuego.obtenerBonusKillAll()->getEstado())
			{
				GranMensaje.append("SBKA"); //Si Bonus KillAll
				GranMensaje.append(";");
				GranMensaje.append(UnJuego.obtenerBonusKillAll()->getInicial());
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.obtenerBonusKillAll()->getX()));
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.obtenerBonusKillAll()->getY()));
				GranMensaje.append(";");
				//UnJuego.bonusYaMostrado();
			}
			else
			{
				GranMensaje.append("NB"); //No Bonus
				GranMensaje.append(";");
			}
			UnJuego.MutexearListaRepuestos();
			GranMensaje.append(IntAString(UnJuego.getRepuestosArma()->getTamanio()));
			GranMensaje.append(";");
			UnJuego.getRepuestosArma()->iniciarCursor();
			while (UnJuego.getRepuestosArma()->avanzarCursor())
			{
				GranMensaje.append(UnJuego.getRepuestosArma()->obtenerCursor()->getInicial());
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.getRepuestosArma()->obtenerCursor()->getX()));
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.getRepuestosArma()->obtenerCursor()->getY()));
				GranMensaje.append(";");
			}
			UnJuego.DesmutexearListaRepuestos();

			Lista<Enemigo *>* enemigos = UnJuego.GetEnemigosPantalla();
			UnJuego.MutexearListaEnemigos();
			enemigos->iniciarCursor();

			GranMensaje.append(IntAString(enemigos->getTamanio()));
			GranMensaje.append(";");
			while (enemigos->avanzarCursor()) {
				Enemigo* UnEnemigo = enemigos->obtenerCursor();
				GranMensaje.append(UnEnemigo->getID());					// ID del sprite (0)
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnEnemigo->getX()));		// PosX(1)
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnEnemigo->getY()));		// PosY(2)
				GranMensaje.append(";");
				GranMensaje.append(UnEnemigo->getEstado());				// Estado(3)
				GranMensaje.append(";");
			}

			UnJuego.DesmutexearListaEnemigos();

			UnJuego.MutexearListaProyectiles();
			Lista<Proyectil *>* Proyectiles = UnJuego.GetProyectiles();
			Proyectiles->iniciarCursor();

			int tmptam = Proyectiles->getTamanio();
			int tmpcuent = 0;

			GranMensaje.append(IntAString(Proyectiles->getTamanio()));
			GranMensaje.append(";");

			while (Proyectiles->avanzarCursor()) {

				Proyectil* UnProyectil = Proyectiles->obtenerCursor();

				GranMensaje.append(UnProyectil->GetIDSprite());
				GranMensaje.append(";");
				GranMensaje.append(UnProyectil->GetEstado());
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnProyectil->GetX()));
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnProyectil->GetY()));
				GranMensaje.append(";");

				tmpcuent++;
			}
			UnJuego.DesmutexearListaProyectiles();
			if (tmpcuent != tmptam) {
				GranMensaje.append(";");
			}


			for(int i = 0;i< UnJuego.GetCantJugadores();i++)
			{
				GranMensaje.append(ObtenerTextoPuntaje(UnJuego.obtenerModo(), i));
				GranMensaje.append(";");
			}

			UnJuego.MutexearListaJugadores();
			int IndiceMiJugador = UnJuego.GetIndiceJugador(Usuario);
			Jugador* MiJugador = UnJuego.GetJugador(IndiceMiJugador);

			for (int i = 0; i < CantJugadores; i++) {

				if (i != IndiceMiJugador) {

					Jugador* OtroJugador = UnJuego.GetJugador(i);

					string Nombre = OtroJugador->GetNombre();
					string IDSprite = OtroJugador->GetIDSprite();
					string Estado = OtroJugador->GetEstado();
					string PosX = IntAString(OtroJugador->GetX());
					string PosY = IntAString(OtroJugador->GetY());

					GranMensaje.append(Nombre);
					GranMensaje.append(";");
					GranMensaje.append(IDSprite);
					GranMensaje.append(";");
					GranMensaje.append(Estado);
					GranMensaje.append(";");
					GranMensaje.append(PosX);
					GranMensaje.append(";");
					GranMensaje.append(PosY);
					GranMensaje.append(";");
				}
			}
			UnJuego.DesmutexearListaJugadores();
			// Renderizo por ultimo mi jugador para asi aparece al frente

			GranMensaje.append(MiJugador->GetNombre().c_str()); // Nombre
			GranMensaje.append(";");
			GranMensaje.append(MiJugador->GetIDSprite().c_str()); // Sprite
			GranMensaje.append(";");
			GranMensaje.append(MiJugador->GetEstado().c_str()); // Estado
			GranMensaje.append(";");
			GranMensaje.append(IntAString(MiJugador->GetX())); // PosX
			GranMensaje.append(";");
			GranMensaje.append(IntAString(MiJugador->GetY())); // PosY
			GranMensaje.append(";");

			GranMensaje.append("Vida: " + IntAString(MiJugador->GetVida()));
			GranMensaje.append(";");
			GranMensaje.append(MiJugador->GetArmaEnUso()->GetNombre() + ": " + IntAString(MiJugador->GetArmaEnUso()->GetBalas()));
			GranMensaje.append(";");

			if (UnJuego.GetEnemigoFinalMurio()) {

				GranMensaje.append("SI");
				GranMensaje.append(";");

				if (UnJuego.GetFinJuego()) {
					GranMensaje.append("SI");
					GranMensaje.append(";");
				}
				else {
					GranMensaje.append("NO");
					GranMensaje.append(";");
				}

				Lista<std::string>* Textos = GetTextosFinNivel(UnJuego.obtenerModo());

				GranMensaje.append(IntAString(Textos->getTamanio()));
				GranMensaje.append(";");

				Textos->iniciarCursor();
				while (Textos->avanzarCursor()) {

					GranMensaje.append(Textos->obtenerCursor());
					GranMensaje.append(";");
				}
			}
			else {

				GranMensaje.append("NO");
				GranMensaje.append(";");
			}

			if (MiJugador->GetVida() <= 0) {

				GranMensaje.append("SI");
				GranMensaje.append(";");
			}
			else {

				GranMensaje.append("NO");
				GranMensaje.append(";");
			}

			UnServer.EnviarMensajeTamanoVariable(GranMensaje, ClientSocket);
			if (CantidadMensajes > 0) {
				Buzon->iniciarCursor();

				while (Buzon->avanzarCursor())
				{
					string ContenidoMensaje = Buzon->obtenerCursor()->obtenerContenido();
					UnServer.EnviarMensajeTamanoVariable(ContenidoMensaje, ClientSocket);
				}
			}

		}
		if (mensaje == "ENVI")
		{
			string destinatario = UnServer.RecibirMensaje(ClientSocket, 15);
			string contenidoMensaje = UnServer.RecibirMensajeTamanoVariable(ClientSocket);

			Mensaje* unMensaje = new Mensaje(Usuario, destinatario, contenidoMensaje);

			WaitForSingleObject(ghMutex, INFINITE);

			UnServer.agregarMensaje(unMensaje);
			
			ReleaseMutex(ghMutex);

			UnServer.EscribirLog("Mensaje enviado con exito, de: " + Usuario + " a " + destinatario + ". Mensaje: " + contenidoMensaje, false);
		}
		if (mensaje == "OUT") {

			if (Usuario != "") {
				UnServer.EnviarMensaje("Hasta la proxima " + Usuario, 40, ClientSocket);
				UnServer.EscribirLog(Usuario + " desconectado correctamente.", true);
				Usuario = "";
			} else {
				UnServer.EnviarMensaje("No existe ninguna sesion iniciada.", 40, ClientSocket);
				UnServer.EscribirLog("Fallo intento de Logout. No existe sesion iniciada.", true);
			}
		}
		if (mensaje == "PING")
		{
			UnServer.EnviarMensaje("OK", 2, ClientSocket);
			EsThreadDePing = true;
		}
		if (mensaje == "ENVT")
		{
			string contenidoMensaje = UnServer.RecibirMensajeTamanoVariable(ClientSocket);
			Lista<std::string>* TodosLosUsuarios = ControlUsuarios.obtenerTodos();

			// Eespera que termine de ejecutar
			WaitForSingleObject(ghMutex, INFINITE);

			UnServer.enviarATodos(contenidoMensaje, Usuario, TodosLosUsuarios);

			ReleaseMutex(ghMutex);

			UnServer.EscribirLog("Mensaje de " + Usuario + " enviado a todos los usuarios. Mensaje: " + contenidoMensaje, true);
		}
		if (mensaje == "REC")
		{
			if (Usuario != "") {
				string respuestaServer = "";
				//Uso del recurso
				WaitForSingleObject(ghMutex, INFINITE); 

				Lista<Mensaje*>* buzon = UnServer.obtenerMensajesPara(Usuario);
				stringstream ss;
				ss << buzon->getTamanio();
				string CantidadMensajes = ss.str();
				UnServer.EnviarMensaje(CantidadMensajes, 8, ClientSocket);

				buzon->iniciarCursor();

				while (buzon->avanzarCursor())
				{
					string UsuarioEmisor = buzon->obtenerCursor()->obtenerEmisor();
					string ContenidoMensaje = buzon->obtenerCursor()->obtenerContenido();

					UnServer.EnviarMensaje(UsuarioEmisor, 15, ClientSocket);
					UnServer.EnviarMensajeTamanoVariable(ContenidoMensaje, ClientSocket);
				}
				ReleaseMutex(ghMutex);
				//Liberacion del recurso
			} else {

				UnServer.EnviarMensaje("NOLOGIN", 8, ClientSocket);
			}
		}
		if (mensaje == "CHKU") {

			if (Usuario == "") {
				UnServer.EnviarMensaje("NO", 2, ClientSocket);
			} else {
				UnServer.EnviarMensaje("SI", 2, ClientSocket);
			}
		}
		// Carga de Sprites
		if (mensaje == "SPRI")
		{
			tinyxml2::XMLDocument docu;

			char* pathXML = strdup(ArchivoEscenarios.c_str());

			if (docu.LoadFile(pathXML) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
			{

				tinyxml2::XMLElement* elementoEscenario = docu.FirstChildElement();

				tinyxml2::XMLElement* elementoSprites = elementoEscenario->FirstChildElement("SPRITES");

				const char* cantidadSrites = elementoSprites->Attribute("cantidad");
				// Envio cantidad de sprites a cargar
				UnServer.EnviarMensajeTamanoVariable(cantidadSrites, ClientSocket);

				//iterando sobre todos los Sprites 
				int contador = 0;
				for (tinyxml2::XMLElement* elementoSprite = elementoSprites->FirstChildElement("SPRITE"); elementoSprite != NULL; elementoSprite = elementoSprite->NextSiblingElement("SPRITE"))
				{

					// Envio info de los sprites a cargar
					const char* idSprite = elementoSprite->Attribute("id");
					UnServer.EnviarMensajeTamanoVariable(idSprite, ClientSocket);	// ID

					const char* frameWidthSprite = elementoSprite->Attribute("frameWidth");
					UnServer.EnviarMensajeTamanoVariable(frameWidthSprite, ClientSocket);			// FRAME WIDTH

					const char* frameHeightSprite = elementoSprite->Attribute("frameHeight");
					UnServer.EnviarMensajeTamanoVariable(frameHeightSprite, ClientSocket);			// FRAME HEIGHT

					const char* velocidadSprite = elementoSprite->Attribute("velocidad");
					UnServer.EnviarMensajeTamanoVariable(velocidadSprite, ClientSocket);			// VELOCIDAD

					DatosSprites* UnSprite = new DatosSprites();
					UnSprite->id = idSprite;
					UnSprite->width = stoi(frameWidthSprite);
					UnSprite->height = stoi(frameHeightSprite);
					UnSprite->velocidad = stoi(velocidadSprite);
					ListaSprites->agregar(UnSprite);

					tinyxml2::XMLElement* elementoEstados = elementoSprite->FirstChildElement("ESTADOS");
					const char* cantidadEstados = elementoEstados->Attribute("cantidad");
					UnServer.EnviarMensajeTamanoVariable(cantidadEstados, ClientSocket);			// CANT ESTADOS

					//iterando sobre todos los estados de un Sprite
					for (tinyxml2::XMLElement* elementoEstado = elementoEstados->FirstChildElement("ESTADO"); elementoEstado != NULL; elementoEstado = elementoEstado->NextSiblingElement("ESTADO"))
					{

						const char* idEstado = elementoEstado->Attribute("id");
						UnServer.EnviarMensajeTamanoVariable(idEstado, ClientSocket);		// NOMBRE ESTADO
						const char* cantFramesEstado = elementoEstado->Attribute("cantFrames");
						UnServer.EnviarMensajeTamanoVariable(cantFramesEstado, ClientSocket);			// CANT FRAMES ESTADO
					}

				}

			}
			else std::cout << "error al cargar el archivo de configuracion de escenariodef.xml " << std::endl;

			UnJuego.SetListaDatosSprites(ListaSprites);
		}
		
		if (mensaje == "NEWC") {

			tinyxml2::XMLDocument docu;
			const char* cantMaxJugadores;
			char* pathXML =  strdup(ArchivoEscenarios.c_str());
			if (docu.LoadFile(pathXML) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
			{
				tinyxml2::XMLElement* elementoEscenario = docu.FirstChildElement();
				 cantMaxJugadores = elementoEscenario->Attribute("maxJugadores");
			}
			
			int cantMax = stoi(cantMaxJugadores);

			if ((CantidadClientes >= cantMax) && (cantMax != NULL)){

				// Si el server alcanzo el maximo de usuarios -> Envio codigo de error
				UnServer.EnviarMensaje("001", 3, ClientSocket);
				UnServer.EnviarMensaje("El servidor alcanzo el maximo de clientes permitidos, vuelva a intentar mas tarde.", 65, ClientSocket);
			} else {
				// Si esta todo ok -> Envio lista de usuarios del server
				string TodosLosUsuarios = ControlUsuarios.obtenerTodosEnString(";");
				UnServer.EnviarMensaje("000", 3, ClientSocket);
				UnServer.EnviarMensajeTamanoVariable(TodosLosUsuarios, ClientSocket);
				CantidadClientes++;
			}
		}

		if (mensaje == "ESCN")
		{
			tinyxml2::XMLDocument docu;

			std::string path = "Archivos\\escenariodef" + IntAString(Nivel) + ".xml";

			if (docu.LoadFile(path.c_str()) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
			{

				tinyxml2::XMLElement* elementoEscenario = docu.FirstChildElement();

				tinyxml2::XMLElement* elementoFondo = elementoEscenario->FirstChildElement("FONDO");

				//iterando sobre todas las capas del fondo 
				for (tinyxml2::XMLElement* elementoCapas = elementoFondo->FirstChildElement("CAPAS"); elementoCapas != NULL; elementoCapas = elementoCapas->NextSiblingElement("CAPAS"))
				{

					const char* cantidadCapasFondo = elementoCapas->Attribute("cantidad");
					// Envio cantidad de capas a cargar
					UnServer.EnviarMensajeTamanoVariable(cantidadCapasFondo, ClientSocket);

					//iterando sobre todos los estados de un Sprite
					for (tinyxml2::XMLElement* elementoImagen = elementoCapas->FirstChildElement("IMAGEN"); elementoImagen != NULL; elementoImagen = elementoImagen->NextSiblingElement("IMAGEN"))
					{

						const char* nombreImagen = elementoImagen->Attribute("nombre");
						UnServer.EnviarMensajeTamanoVariable(nombreImagen, ClientSocket);		// NOMBRE Imagen de la capa

						const char* zIndex = elementoImagen->Attribute("zIndex");
						UnServer.EnviarMensajeTamanoVariable(zIndex, ClientSocket);			// zIndex de la capa en el fondo
						
						const char* ancho = elementoImagen->Attribute("width");
						UnServer.EnviarMensajeTamanoVariable(ancho, ClientSocket);

						const char* alto = elementoImagen->Attribute("height");
						UnServer.EnviarMensajeTamanoVariable(alto, ClientSocket);
					}

				}
			}
			else std::cout << "error al cargar el archivo de configuracion de escenario.xml " << std::endl;
		}

	}

	if (!EsThreadDePing) { // Cliente se desconecto

		if (Usuario != "") {

			UnJuego.GetJugador(Usuario)->SetEstaConectado(false);
			Lista<std::string>* UsuariosOnline = UnJuego.GetNombresJugadoresOnline();
			std::string Mensaje = Usuario + " se desconecto";
			UnServer.enviarATodos(Mensaje, Usuario, UsuariosOnline);
		}

		CantidadClientes--;

		if (mensaje == "EXIT") {
			UnServer.EscribirLog("Un cliente se desconecto", true);
		}
		else {
			UnServer.EscribirLog("Un cliente perdio la conexion", true);
		}
	}

	closesocket(ClientSocket);
}

void MainServerThread(void* arg) {
	ghMutex = CreateMutex(NULL,FALSE,NULL);
	string Puerto = *(string*)arg;
	UnServer.Abrir(Puerto);

	while (true) {
		SOCKET ClientSocket = UnServer.RecibirNuevaConexion();
		_beginthread(MainListenThread, 0, (void*)&ClientSocket);
	}

}

void MenuPrincipal() {
	string comando;

	while (true)
	{
		MostrarListaComandos();
		cin >> comando;
		if (comando == "q")
		{
			cout << "Servidor off\n";
			break;
		}
	}
}

int main()
{

	cout << "Ingrese el puerto donde escuchara conexiones el servidor: ";
	string Puerto;

	std::getline(std::cin, Puerto);

	if (Puerto.empty())
	{
		Puerto = "1000";
		cout << endl << "Se usara el puerto por default: 1000" << endl;
	}

	string NombreArchivoUsuarios = "";

	while (!ControlUsuarios.SetNombreArchivo(NombreArchivoUsuarios)) {

		cout << endl << "Ingrese el nombre del archivo de usuarios: ";
		std::getline(std::cin, NombreArchivoUsuarios);

		if (NombreArchivoUsuarios.empty())
		{
			NombreArchivoUsuarios = "Usuarios.csv";
			cout << "Se usara el archivo por default: Usuarios.csv" << endl;
		}
	}

	string pathEscenario = "";
	//while (!ControlUsuarios.SetNombreArchivo(pathEscenario))
	while(!std::ifstream("Archivos\\" + pathEscenario))
	{
		cout << endl << "Ingrese el nombre del archivo de escenario: ";
		std::getline(std::cin, pathEscenario);

		if (pathEscenario.empty())
		{
			pathEscenario = "escenariodef" + IntAString(Nivel) + ".xml";
			cout << "Se usara el archivo por default: escenariodef1.xml" << endl;
		}
	}
	
	ArchivoEscenarios = "Archivos\\" + pathEscenario;

	CargarPlataformas();
	CargarEscenariosEnJuego();
	CargarSprites();
	CargarEnemigos();
	
	PedirModoDeJuego();

	// Thread 1: Inicializacion del server

	_beginthread(MainServerThread, 0, (void*)&Puerto);

	// Thread 0: Handler de eventos del server
	Sleep(1000);
	MenuPrincipal();

	return 0;
}
