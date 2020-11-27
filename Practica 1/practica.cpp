/**
 * Actualmente este fichero lo que hace es un ls
*/

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios
#include <cmath>

char *origen;  //El path origen que me han pasado
char *destino; //El path origen que me han pasado
int op = -1;
const char *BM = "BM";
const int RESERVADO = 0;
const int BYTEPORPIXEL = 3;
const int SINFOCABECERA = 40;
const int SCABECERA = 14;
const int OFFSETIMAGEN = SINFOCABECERA + SCABECERA;
const short NPLANOS = 1;
const short BITSPORPIXEL = BYTEPORPIXEL * 8;
const short INFOSHORT[] = {NPLANOS, BITSPORPIXEL};
const int COMPRESION = 0;
const int RESOLUCIONX = 11811; //300 dpi
const int RESOLUCIONY = 11811; //300 dpi
const int SCOLOR = 0;
const int COLORESIMPORTANTES = 0;
using namespace std;

char *obtenerFilePath(char *path, char *fichero);
int operacion(char *fichero);
unsigned char *leerImagen(const char *fileName, int *anchura, int *altura);
void escribirImagen(const char *fileName, unsigned char *pixelsImagen, int anchura, int altura);
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
    //Obtener la operación pasada por argumento
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

    origen = argv[2];                   // El path origen que me han pasado
    destino = argv[3];                  // El path destino que me han pasado
    struct dirent *eDirOrigen;          // Lee los ficheros que hay en el directorio de origen
    DIR *dirOrigen = opendir(origen);   // Obtengo todos los ficheros del origen
    DIR *dirDestino = opendir(destino); // Obtengo todos los ficheros del destino
    // Debe de existir los dos directorios
    if (dirOrigen == NULL || dirDestino == NULL)
    {
        perror("No existe uno de los directorios que has pasado por argumento");
        return -1;
    }
    if (origen[strlen(origen) - 1] != '/')
        strcat(origen, "/"); // En el caso de que no exista la barra en dir origen
    if (destino[strlen(destino) - 1] != '/')
        strcat(destino, "/"); // En el caso de que no exista la barra en dir origen

    while ((eDirOrigen = readdir(dirOrigen)) != NULL) // Mientras el elemento que me pase el directorio no sea nulo
    {
        if (strcmp(eDirOrigen->d_name, ".") && strcmp(eDirOrigen->d_name, "..")) // Evito que utilicen como fichero el . y ..
        {
            if (operacion(eDirOrigen->d_name) == -1) // Tareas de la imagen
                return -1;
        }
    }
    return 0;
}

/* Esta función realiza la acción indicada por el usuario a cada uno de los archivos */
int operacion(char *fichero)
{
    char *filePathOrigen = obtenerFilePath(origen, fichero);
    int anchura = 0;
    int altura = 0;
    unsigned char *imagenOrigen = leerImagen(filePathOrigen, &anchura, &altura);
    free(filePathOrigen);
    int size = anchura * altura * 3;
    unsigned char *imagenDestino;

    switch (op)
    {
    case 1:
        imagenDestino = imagenOrigen;
        break;
    case 2:
        imagenDestino = gauss(anchura, altura, size, imagenOrigen);
        break;
    case 3:
        imagenDestino = sobel(anchura, altura, size, imagenOrigen);
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
    escribirImagen(filePathDestino, imagenDestino, anchura, altura);
    free(imagenOrigen);
    free(filePathDestino);
    return 0;
}

/* Esta funcion obtiene el path donde se encuentra el fichero juntando la carpeta origen y el nombre del archivo */
char *obtenerFilePath(char *path, char *fichero)
{
    char *filePath = (char *)malloc(256);        // Creo un espacio donde guardar los paths a los archivos
    memcpy(filePath, path, strlen(path));        // Copio la carpeta
    strncat(filePath, fichero, strlen(fichero)); // Copio el nombre del fichero
    return filePath;                             // Devuelvo el puntero al path completo hacia el archivo
}

/* Esta función lee la imagen que ha recibido por parámetro y comprueba que todos los parámetros necesarios 
   son correctos. También actualiza los valores anchura y altura pasados por parámetro*/
unsigned char *leerImagen(const char *fileName, int *anchura, int *altura)
{
    FILE *leerDF = fopen(fileName, "rb"); // Descriptor de fichero de la imagen
    int dataOffset;
    // Obtener desde donde empieza la imagen, la anchura y la altura
    fseek(leerDF, 10, SEEK_SET); // Posición del valor de donde empieza la imagen
    fread(&dataOffset, 4, 1, leerDF);
    fseek(leerDF, 18, SEEK_SET); // Posición del valor de la anchura
    fread(anchura, 4, 1, leerDF);
    fread(altura, 4, 1, leerDF);
    // Un BMP está escrito alrevés, es decir, al principio están los pixeles finales y al final los del principio
    // Lo que vamos a hacer es obtener la imagen al derecho al leerla:
    int anchuraTotal = (*anchura) * BYTEPORPIXEL;                                  // Lo que ocupa cada linea de la imagen
    int size = anchuraTotal * (*altura);                                           // Tamano total de la parte de imagen, ya que cada pixel son 3 bytes
    unsigned char *datosImagen = (unsigned char *)malloc(size);                    // Obtengo espacio para la imagen
    unsigned char *posicionPuntero = datosImagen + ((*altura - 1) * anchuraTotal); // Leo primero los ultimos bytes
    for (int i = 0; i < *altura; i++)
    {
        fseek(leerDF, dataOffset + (i * anchuraTotal), SEEK_SET); // Posiciono el cursor para leer la imagen
        fread(posicionPuntero, 1, anchuraTotal, leerDF);          // Leo por lineas
        posicionPuntero -= anchuraTotal;                          // Actualizo la linea siguiente
    }
    fclose(leerDF); // Cierro el descriptor de fichero
    return datosImagen;
}

/* Esta funcion obtiene todos los parametros necesarios para leer una imagen y escribe la imagen pasada por argumento
   ademas del lugar donde guardarla, la altura y la anchura*/
void escribirImagen(const char *fileName, unsigned char *pixelsImagen, int anchura, int altura)
{
    FILE *escribirDF = fopen(fileName, "wb");
    int sTotal, anchuraTotal, sImagen;
    anchuraTotal = (anchura)*3;
    sTotal = anchuraTotal * altura + SCABECERA + SINFOCABECERA;
    sImagen = anchura * altura * BYTEPORPIXEL;
    int intInfoHeader[] = {sTotal, RESERVADO, OFFSETIMAGEN, SINFOCABECERA, anchura, altura, COMPRESION,
                           sImagen, RESOLUCIONX, RESOLUCIONY, SCOLOR, COLORESIMPORTANTES};
    // Escribir cada uno de los parámetros de la cabecera
    fwrite(BM, 1, 2, escribirDF);
    fwrite(intInfoHeader, 4, 6, escribirDF);
    fwrite(INFOSHORT, 2, 2, escribirDF);
    fwrite(&intInfoHeader[6], 4, 6, escribirDF);
    // Escribir la imagen de la cabecera de
    for (int i = altura; i > 0; i--)
    {
        int pixelOffset = (i - 1) * anchuraTotal;
        fwrite(&pixelsImagen[pixelOffset], 1, anchuraTotal, escribirDF);
    }
    fclose(escribirDF);
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