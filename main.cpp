#include <iostream>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <vector>
#include <pugixml.hpp>

#define HOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "salvadorgroc"
#define DATABASE "dbgame"

#ifndef DEBUG
    #define PUGIXML_HEADER_ONLY
#endif

#include <pugixml.hpp>

using namespace std;

pugi::xml_document doc;

int
iIdJugador,
iIdRaza;

std::string
sUserNick,
sUserPass,
sUserPassRepeat,
sUserRaces,
sCharacterName,
sPlayerPosition;

std::vector<string> sNameRaces;

bool
bVerified = false,
bPlayerCreated = false,
bRepeatPassword = false,
bRaceCreated = false,
bCharacterCreated = false;

int main()
{
    try
    {

        //Conectamos a base de datos.
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);
        sql::Statement* stmt = con->createStatement();

        while(!bVerified)
        {

            //Preguntamos nombre de usuario
            std::cout << "Inserte nombre de usuario: " << std::endl;
            std::cin >> sUserNick;

            //Comprobamos si existe el nick del jugador
            sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "'");


            if(res->next() && res->getInt(1) == 1) //Existe usuario
            {
                //Pedimos contraseña
                std::cout << "\nIntroduzca contraseña" << std::endl;
                std::cin >> sUserPass;

                //Comprobamos si existe usuario con dichos datos
                sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "' AND Pass = '" + sUserPass + "'");

                if(res->next() && res->getInt(1) == 1) //Existe usuario con contraseña
                {
                    //Se han validado los datos y puede iniciar el juego.
                    bVerified = true;
                }
                else // No existe usuario con contraseña
                {
                    std::cout << "\nNo se ha encontrado ningun usuario con esa contraseña.\n" << std::endl;
                }
            }
            else //No existe usuario
            {
                std::cout << "\nNo existe dicho usuario. Se va a proceder a crear un nuevo usuario.\n" << "\nInserte nombre de usuario" <<std::endl;
                while(!bPlayerCreated)//Mientras no este el jugador creado repetiremos
                {
                    //Pedimos al usuario que indique un nuevo usuario

                    std::cin >> sUserNick;

                    //Comprobamos si el nick esta libre
                    sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "'");
                    if(res->next() && res->getInt(1) == 1) //Existe usuario
                    {
                        std::cout << "Usuario ya en uso, inserte otro nombre de usuario." << std::endl;
                    }
                    else  //No existe usuario
                    {
                        std::cout << "\nUsuario disponible, inserte contraseña.\n";
                        while(!bRepeatPassword)//Repetimos hasta tener una contraseña valida
                        {
                            //Pedimos al usuario que escriba y repita la contraseña
                            std::cin >> sUserPass;
                            std::cout << "\nRepita contraseña.\n";
                            std::cin >> sUserPassRepeat;

                            if(sUserPass == sUserPassRepeat)//Las contraseñas coinciden
                            {

                                stmt->execute("INSERT INTO Jugadores (Nombre, Pass) VALUES ('"+ sUserNick +"', '"+ sUserPass +"')");
                                bRepeatPassword = true;
                                bPlayerCreated = true;
                            }
                            else //Las contraseñas no coinciden
                            {
                                std::cout << "\nLas contraseñas no coinciden, introduzca de nuevo la contraseña.\n";

                            }
                        }

                        //Creacion de raza
                        system("clear");

                        //Listamos las razas
                        res = stmt->executeQuery("SELECT Nombre, Descripcion FROM Razas");
                        std::cout<<"Nombre     |      Descripcion\n"<<std::endl;
                        while(res->next())
                        {
                            sNameRaces.push_back(res->getString("Nombre"));
                            std::cout<<res->getString("Nombre")<<"      |      "<<res->getString("Descripcion")<<std::endl;
                        }
                        std::cout << "\nSelecciona una raza.\n";

                        //Preguntaremos razas hasta que el usuario introduzca una disponible
                        while(!bRaceCreated)
                        {
                            std::cin >> sUserRaces;
                            for(int i = 0; i < sNameRaces.size(); i++)
                            {
                                if(sUserRaces == sNameRaces[i])
                                {
                                    bRaceCreated = true;

                                    break;
                                }
                                else if(i == 2)
                                {

                                    std::cout << "\nRaza no encontrada, seleccione una raza de la lista." << std::endl;

                                }
                            }
                        }

                        std::cout << "\nInserte nombre del personaje." << std::endl;
                        while(!bCharacterCreated)
                        {
                            std::cin >> sCharacterName;
                            res = stmt->executeQuery("SELECT count(*) FROM Personajes, Jugadores, Razas WHERE Personajes.IDJugador = Jugadores.JugadorID AND Personajes.IDRaza = Razas.RazaID AND Personajes.Nombre = '"+sCharacterName+"'");
                            if(res->next() && res->getInt(1) == 1) //Existe personaje
                            {
                                std::cout << "\nNombre ya en uso, inserte otro nombre de personaje." << std::endl;
                            }
                            else //Nombre libre
                            {
                                bCharacterCreated = true;
                                //Obtenemos id del jugador
                                res = stmt->executeQuery("SELECT JugadorId FROM Jugadores WHERE Nombre = '"+sUserNick+"'");
                                while(res->next())
                                {

                                    iIdJugador = res->getInt("JugadorID");
                                }

                                //Obtenemos id de la raza
                                res = stmt->executeQuery("SELECT RazaId FROM Razas WHERE Nombre = '"+sUserRaces+"'");
                                while(res->next())
                                {

                                    iIdRaza = res->getInt("RazaID");
                                }

                                //Insertamos el personaje en la base de datos
                                stmt->execute("INSERT INTO Personajes(Nombre, IDJugador, IDRaza) VALUES ('"+sCharacterName+"', "+ std::to_string(iIdJugador) +", "+ std::to_string(iIdRaza) +")");

                                bVerified = true;
                            }
                        }
                    }
                }
            }
        }

        system("clear");

        std::cout << "Empieza el juego.";

        //Cargamos archivo xml
        pugi::xml_parse_result result = doc.load_file("Mazmorra.xml");
        pugi::xml_node currentNode = doc.child("mazmorra");

        //Nos desplazamos hasta la sala inicial
        currentNode = currentNode.child("habitacion");
        sPlayerPosition = currentNode.attribute("id").value();


        std::cout << sPlayerPosition << std::endl;






    }

    catch(sql::SQLException &e)
    {
        std::cout << "Se produce el error " << e.getErrorCode()<<std::endl;
    }
    return 0;
}
