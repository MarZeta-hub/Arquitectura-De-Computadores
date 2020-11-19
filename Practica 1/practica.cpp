/**
 * Actualmente este fichero lo que hace es un ls
*/

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios

using namespace std;

int main(int argc, char *argv[])
{

    //Si el número de argumentos que me pasa el programa es menor que 4 es que está mal
    if (argc != 4)
    {
        perror("El número de parámetros no es correcto");
        return -1;
    }

    //En el caso de que no sea ninguno de estos el tipo pasado por argumento
    if (strcmp(argv[1], "copy") && strcmp(argv[1], "gauss") && strcmp(argv[1], "sobel"))
    {
        perror("No existe ese tipo");
        return -1;
    }

    struct dirent *eDirOrigen;          //Lee los ficheros que hay en el Directorio de origen
    DIR *dirOrigen = opendir(argv[2]);  //Obtengo todos los ficheros del origen
    DIR *dirDestino = opendir(argv[2]); //Obtengo todos los ficheros del destino

    //Debe de existir los dos directorios
    if (dirOrigen != NULL && dirDestino != NULL)
    {
        //Navego dentro del primer directorio
        while (((eDirOrigen = readdir(dirOrigen)) != NULL))
        {
            cout << eDirOrigen->d_name << "\n"; //Hace el ls del directorio que le has pasado
        }
    }
    else
    {
        perror("No existe uno de los directorios que has pasado por argumento");
        return -1;
    }

    return 0;
}