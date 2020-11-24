/**
 * Actualmente este fichero lo que hace es un ls
*/

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios

using namespace std;

int operation(char *fichero);
int comprobarBMP(char *fichero);
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
    char *origen = argv[2];             //El path origen que me han pasado
    char *destino = argv[3];            //El path origen que me han pasado
    struct dirent *eDirOrigen;          //Lee los ficheros que hay en el Directorio de origen
    DIR *dirOrigen = opendir(origen);   //Obtengo todos los ficheros del origen
    DIR *dirDestino = opendir(destino); //Obtengo todos los ficheros del destino
    //Debe de existir los dos directorios
    if (dirOrigen == NULL || dirDestino == NULL)
    {
        perror("No existe uno de los directorios que has pasado por argumento");
        return -1;
    }
    if (origen[strlen(origen) - 1] != '/')
        strcat(origen, "/"); //En el caso de que no exista la barra en dir origen
    if (destino[strlen(destino) - 1] != '/')
        strcat(destino, "/");                         //En el caso de que no exista la barra en dir origen

    while ((eDirOrigen = readdir(dirOrigen)) != NULL) //Mientras el elemento que me pase el directorio no sea nulo
    {
        if (strcmp(eDirOrigen->d_name, ".") && strcmp(eDirOrigen->d_name, "..")) //Evito que utilicen como fichero el . y ..
        {
            char *filePath = new char[0];
            strcat(filePath, origen);                  //Copio al filePath la carpeta de origen
            strcat(filePath,eDirOrigen->d_name);       //Copio el nombre del fichero
            comprobarBMP(filePath);                    //Comprobar si es un BMP
            operation(filePath);                       //realizo la operación que me ha pedido
        }
    }
    return 0;
}

int operation(char *fichero)
{
    cout << fichero << "\n";
    return 0;
}

int comprobarBMP(char *fichero)
{
    FILE *fd = fopen(fichero, "rb");
    unsigned char info[54];
    fread(info, sizeof(unsigned char), 54, fd); // read the 54-byte header
    if (info[0] != 'B' || info[1] != 'M')
    {
        cout << "El archivo insertado no es un .bmp\n";
        return -1;
    }
    else if (*(char *)&info[26] != 1)
    {
        cout << "El número de planos es superior a lo establecido (default = 1)\n";
        return -1;
    }
    else if (*(char *)&info[28] != 24)
    {
        cout << "El tamaño por punto es diferente a lo establecido (default = 24)\n";
        return -1;
    }
    else if (*(char *)&info[30] != 0)
    {
        cout << "La compresión no es correcta (default = 0)\n";
        return -1;
    }
    else
    {
        cout << "Tipo de archivo correcto\n";
    }
    return 0;
}