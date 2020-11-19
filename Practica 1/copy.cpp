/**
 * Actualmente este fichero lo que hace es un ls
*/

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;
int operation(char *fichero);

int main(int argc, char *argv[])
{
    char origen[8] = "origen";
    strcat(origen, "/");
    char destino[9] = "destino/";
    strcat(destino, "/");

    struct dirent *eDirOrigen;          //Lee los ficheros que hay en el Directorio de origen
    DIR *dirOrigen = opendir(origen);   //Obtengo todos los ficheros del origen
    DIR *dirDestino = opendir(destino); //Obtengo todos los ficheros del destino
    char ficheroPath[strlen(origen)];   //para obtener el path del fichero según cada elemento

    //Mientras existan ficheros en la carpeta
    while ((eDirOrigen = readdir(dirOrigen)) != NULL)
    {
        //Obtengo el lugar de la carpeta que me han pasado
        memcpy(ficheroPath, origen, strlen(origen));
        //Evito que utilicen como fichero el . y ..
        if ( strcmp(eDirOrigen->d_name, ".") && strcmp(eDirOrigen->d_name, "..") )
        {
            cout << eDirOrigen->d_name << "\n"; //Hace el ls del directorio que le has pasado sin puntos 
            strcat(ficheroPath, eDirOrigen->d_name); //copio el fichero al final del path
            cout<<ficheroPath<<"\n"; //comprobar que lo realiza bien
            operation(ficheroPath); //realizo la operación que me ha pedido
        }
    }
    return 0;
}

int operation(char *fichero){
   




    return 0;
}