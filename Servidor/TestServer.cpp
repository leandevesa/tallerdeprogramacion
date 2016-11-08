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

string ArchivoEscenarios = "";

HANDLE ghMutex;
Juego UnJuego;

void MostrarListaComandos() {
	cout << "Ingrese la letra ""q"" si desea apagar el servidor: ";
}
bool paso = false;
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
void CargarEscenariosEnJuego() {
	tinyxml2::XMLDocument docu;

	UnJuego.BorrarCamaras();

	char* pathXML = strdup(ArchivoEscenarios.c_str());

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

					std::string IDSprite;

					// TODO: Ver como determinar que sprite mandarle a cada player
					/*if (UnJuego.GetCantJugadores() == 0) {
						IDSprite = "PlayerRed";
					} 
					else {
						if (UnJuego.GetCantJugadores() == 1) {
							IDSprite = "PlayerYellow";
						} else {
							IDSprite = "PlayerBlue";
						}
					}*/

					int cantJugadores = UnJuego.GetCantJugadores();

					switch (cantJugadores)
					{
						case 0:
							IDSprite = "PlayerRed";
							break;
						case 1:
							IDSprite = "PlayerYellow";
							break;
						case 2:
							IDSprite = "PlayerViolet";
							break;
						case 3: 
							IDSprite = "PlayerBlue";
							break;
						default:
							IDSprite = "PlayerBlue";
					}

					UnJuego.AgregarJugador(Usuario, IDSprite);

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
			UnJuego.RecibirEvento(Usuario, Tipo);
			if (Tipo == "RECARGA") {
				CargarEscenariosEnJuego();
			}
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
			if(UnJuego.GetCantJugadores() != 1)
			{
				UnServer.EnviarMensajeTamanoVariable("NOLISTO",ClientSocket);
			}
			else
			{
				UnServer.EnviarMensajeTamanoVariable("OK",ClientSocket);
			}
		}
		if (mensaje == "STAT") {

			std::string GranMensaje = "";

			int CantCamaras = UnJuego.GetCantCamaras();

			for (int i = 0; i < CantCamaras; i++) {

				std::string CamaraX = IntAString(UnJuego.GetCamara(i)->X);
				GranMensaje.append(CamaraX);
				GranMensaje.append(";");
			}
			
			int CantJugadores = UnJuego.GetCantJugadores();
			std::string StrCantJugadores = IntAString(CantJugadores);

			GranMensaje.append(StrCantJugadores);
			GranMensaje.append(";");

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
			
			// Si hay mensajes para el usuario -> le envio
			Lista<Mensaje*>* Buzon = UnServer.obtenerMensajesPara(Usuario);
			int CantidadMensajes = Buzon->getTamanio();

			GranMensaje.append(IntAString(CantidadMensajes));

			GranMensaje.append(";");
			if(UnJuego.GetCamara(0)->X == 0 && !paso)
			{
				paso = true;
				UnJuego.AgregarEnemigo("PulpoEnemigo",800,300,15,100);
				UnJuego.AgregarEnemigo("HumanoEnemigo", 800, 390,10,100);
			}
			
			//Envio informacion de los enemigos
			//Cantidad de enemigos que aparecen

			GranMensaje.append(IntAString(UnJuego.GetCantEnemigos()));
			GranMensaje.append(";");

			for(int i = 0; i < UnJuego.GetCantEnemigos(); i++)
			{
				GranMensaje.append(UnJuego.GetEnemigo(i)->getID());					// ID del sprite (0)
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.GetEnemigo(i)->getX()));		// PosX(1)
				GranMensaje.append(";");
				GranMensaje.append(IntAString(UnJuego.GetEnemigo(i)->getY()));		// PosY(2)
				GranMensaje.append(";");
				GranMensaje.append(UnJuego.GetEnemigo(i)->getEstado());	// Estado(3)

				if (i != UnJuego.GetCantEnemigos()) 
				{
					GranMensaje.append(";");
				}
			}

			Lista<Proyectil *>* Proyectiles = UnJuego.GetProyectiles();
			Proyectiles->iniciarCursor();

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
				for (tinyxml2::XMLElement* elementoSprite = elementoSprites->FirstChildElement("SPRITE"); elementoSprite != NULL; elementoSprite = elementoSprite->NextSiblingElement("SPRITE"))
				{

					// Envio info de los sprites a cargar
					const char* idSprite = elementoSprite->Attribute("id");
					UnServer.EnviarMensajeTamanoVariable(idSprite, ClientSocket);	// ID
					const char* frameWidthSprite = elementoSprite->Attribute("frameWidth");
					UnServer.EnviarMensajeTamanoVariable(frameWidthSprite, ClientSocket);			// FRAME WIDTH
					const char* frameHeightSprite = elementoSprite->Attribute("frameHeight");
					UnServer.EnviarMensajeTamanoVariable(frameHeightSprite, ClientSocket);			// FRAME HEIGHT

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
		}
		if (mensaje == "NEWC") {

			tinyxml2::XMLDocument docu;

			char* pathXML =  strdup(ArchivoEscenarios.c_str());
			const char* cantMaxJugadores;

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

			char* pathXML =  strdup(ArchivoEscenarios.c_str());

			if (docu.LoadFile(pathXML) != tinyxml2::XML_ERROR_FILE_NOT_FOUND)
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
			pathEscenario = "escenariodef.xml";
			cout << "Se usara el archivo por default: escenariodef.xml" << endl;
		}
	}
	
	ArchivoEscenarios = "Archivos\\" + pathEscenario;

	CargarEscenariosEnJuego();
	
	PedirModoDeJuego();

	// Thread 1: Inicializacion del server

	_beginthread(MainServerThread, 0, (void*)&Puerto);

	// Thread 0: Handler de eventos del server
	Sleep(1000);
	MenuPrincipal();

	return 0;
}
