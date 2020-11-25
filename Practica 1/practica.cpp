/**
 * Actualmente este fichero lo que hace es un ls
*/

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios

char *origen;  //El path origen que me han pasado
char *destino; //El path origen que me han pasado
int op = -1;
using namespace std;

char *obtenerFilePath(char *path, char *fichero);
int comprobarBMP(unsigned char *info);
int operacion(char *fichero);
unsigned char *gauss(int anchura, int altura, int size, unsigned char *imagenOrigen);
unsigned char *sobel(int anchura, int altura, int size, unsigned char *imagenOrigen);

int main(int argc, char *argv[])
{
    //Si el número de argumentos que me pasa el programa es menor que 4 es que está mal
    if (argc != 4)
    {
        perror("El número de parámetros no es correcto");
        return -1;
    }
    //En el caso de que no sea ninguno de estos el tipo pasado por argumento
    if (strcmp(argv[1], "copy") == 0)
    {
        op = 1;
    }
    else if (strcmp(argv[1], "gauss") == 0)
    {
        op = 2;
    }
    else if (strcmp(argv[1], "sobel") == 0)
    {
        op = 3;
    }
    else
    {
        perror("No se ha establecido un operador valido");
        return -1;
    }

    origen = argv[2];                   //El path origen que me han pasado
    destino = argv[3];                  //El path origen que me han pasado
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
        strcat(destino, "/"); //En el caso de que no exista la barra en dir origen

    while ((eDirOrigen = readdir(dirOrigen)) != NULL) //Mientras el elemento que me pase el directorio no sea nulo
    {
        if (strcmp(eDirOrigen->d_name, ".") && strcmp(eDirOrigen->d_name, "..")) //Evito que utilicen como fichero el . y ..
        {
            if (operacion(eDirOrigen->d_name) == -1)//Tareas de la imagen
                return -1; 
        }
    }
    return 0;
}

/*Esta función realiza la acción indicada por el usuario a cada uno de los archivos*/
int operacion(char *fichero)
{
    char *filePathOrigen = obtenerFilePath(origen, fichero);        //para obtener el path hacia el archivo
    int sInfo = 59;                                                 //datos en la cabecera de BMP
    FILE *leer = fopen(filePathOrigen, "rb");                       //obtengo el file descriptor del archivo
    free(filePathOrigen);                                           //libero el malloc realizado para obtener el archivo
    if (leer == NULL)
    {
        perror("Ha dado error la lectura del fichero");
        return -1;
    }
    unsigned char info[59];
    fread(info, sizeof(unsigned char), sInfo, leer);
    if (comprobarBMP(info) == -1)
        return -1;
    int width = *(int *)&info[18];
    int height = *(int *)&info[22];
    int size = 3 * width * height;
    unsigned char *imagenOrigen = new unsigned char[size]; // allocate 3 bytes per pixel
    if ((fread(imagenOrigen, sizeof(unsigned char), size, leer)) == 0)
    {
        perror("Error en la lectura de la imagen");
        fclose(leer);
        return -1;
    }
    fclose(leer);
    
    unsigned char *imagenDestino;
    switch (op)
    {
    case 1:

        imagenDestino = imagenOrigen;
        break;
    case 2:
        imagenDestino = gauss(width, height, size, imagenOrigen);
        break;
    case 3:
        imagenDestino = sobel(width, height, size, imagenOrigen);
        break;
    default:
        perror("El programa nunca debería llegar aqui");
        return -1;
    }

    if (imagenDestino == NULL)
    {
        perror("La imagen generada ha fallado");
        return -1;
    }

    char *filePathDestino = obtenerFilePath(destino, fichero);
    FILE *escribir = fopen(filePathDestino, "wb");
    free(filePathDestino);
    if ((fwrite(info, sizeof(unsigned char), 59, escribir)) == 0)
    {
        perror("Error al escribir la cabecera");
        return -1;
    }
    if ((fwrite(imagenDestino, sizeof(unsigned char), size, escribir)) == 0)
    {
        perror("Error al escribir el contenido de la imagen");
        return -1;
    }

    fclose(escribir);
    return 0;
}

/*Esta función comprueba */
int comprobarBMP(unsigned char *info)
{

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
    return 0;
}

char *obtenerFilePath(char *path, char *fichero)
{
    char *filePath = (char *) malloc(256);          //creo un espacio donde guardar los paths a los archivos
    memcpy(filePath, path, strlen(path));           //copio la carpeta
    strncat(filePath, fichero, strlen(fichero));    //Copio el nombre del fichero
    return filePath;                                //devuelvo el puntero al path completo hacia el archivo
}

unsigned char *gauss(int anchura, int altura, int size, unsigned char *imagenOrigen)
{
    cout << "utilizando Gauss valor de anchura y tal" << anchura << altura << size << "\n";

    return imagenOrigen;
}

unsigned char *sobel(int anchura, int altura, int size, unsigned char *imagenOrigen)
{
    cout << "utilizando Sobel valor de anchura y tal" << anchura << altura << size << "\n";
    return imagenOrigen;
}