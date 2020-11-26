/**
 * Actualmente este fichero lo que hace es un ls
*/

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios
#include <cmath>
#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

char *origen;  //El path origen que me han pasado
char *destino; //El path origen que me han pasado
int op = -1;
using namespace std;

char *obtenerFilePath(char *path, char *fichero);
int comprobarBMP(unsigned char *info);
int operacion(char *fichero);
unsigned char *gauss(int anchura, int altura, int size, unsigned char *imagenOrigen);
unsigned char *sobel(int anchura, int altura, int size, unsigned char *imagenOrigen);
unsigned char *leerImagen(const char *fileName, int *width, int *height);
void escribirImagen(const char *fileName, unsigned char *pixels, int width, int height);

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
            if (operacion(eDirOrigen->d_name) == -1) //Tareas de la imagen
                return -1;
        }
    }
    return 0;
}

/*Esta función realiza la acción indicada por el usuario a cada uno de los archivos*/
int operacion(char *fichero)
{
    cout << fichero;
    char *filePathOrigen = obtenerFilePath(origen, fichero);
    int width = 0;
    int height = 0;
    unsigned char *imagenOrigen = leerImagen(filePathOrigen, &width, &height);
    free(filePathOrigen);
    int size = width * height * 3;
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

    escribirImagen(filePathDestino, imagenDestino, width, height);
    free(imagenOrigen);
    free(filePathDestino);
    return 0;
}

char *obtenerFilePath(char *path, char *fichero)
{
    char *filePath = (char *)malloc(256);        //creo un espacio donde guardar los paths a los archivos
    memcpy(filePath, path, strlen(path));        //copio la carpeta
    strncat(filePath, fichero, strlen(fichero)); //Copio el nombre del fichero
    return filePath;                             //devuelvo el puntero al path completo hacia el archivo
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

/* Esta función lee la imagen que ha recibido por parámetro y comprueba que todos los parámetros necesarios 
   son correctos. También actualiza los valores anchura y altura pasados por parámetro*/
unsigned char *leerImagen(const char *fileName, int *anchura, int *altura)
{
    FILE *df = fopen(fileName, "rb");                                              // Descriptor de fichero de la imagen
    int dataOffset;        
    // Obtener desde donde empieza la imagen, la anchura y la altura                                                     
    fseek(df, 10, SEEK_SET);                                                       // Posición del valor de donde empieza la imagen
    fread(&dataOffset, 4, 1, df);                                                  
    fseek(df, 18, SEEK_SET);                                                       // Posición del valor de la anchura
    fread(anchura, 4, 1, df);                                                      
    fread(altura, 4, 1, df);           
    //Un BMP está escrito alrevés, es decir, al principio están los pixeles finales y al final los del principio
    //Lo que vamos a hacer es obtener la imagen al derecho al leerla:                                           
    int anchuraTotal = (*anchura) * 3;                                             // Lo que ocupa cada linea de la imagen
    int size = anchuraTotal * (*altura);                                           // Tamano total de la parte de imagen, ya que cada pixel son 3 bytes
    unsigned char *datosImagen = (unsigned char *)malloc(size);                    // Obtengo espacio para la imagen
    unsigned char *posicionPuntero = datosImagen + ((*altura - 1) * anchuraTotal); // Leo primero los ultimos bytes
    for (int i = 0; i < *altura; i++)
    {
        fseek(df, dataOffset + (i * anchuraTotal), SEEK_SET);                      // Posiciono el cursor para leer la imagen
        fread(posicionPuntero, 1, anchuraTotal, df);                               // Leo por lineas
        posicionPuntero -= anchuraTotal;                                           // Actualizo la linea siguiente
    }
    fclose(df);                                                                    // Cierro el descriptor de fichero
    return datosImagen;                                                            // Devuelvo la imagen
}

void escribirImagen(const char *fileName, unsigned char *pixels, int width, int height)
{
    FILE *outputFile = fopen(fileName, "wb");
    int bytesPerPixel = 3;
    const char *BM = "BM";
    fwrite(&BM[0], 1, 1, outputFile);
    fwrite(&BM[1], 1, 1, outputFile);
    int paddedRowSize = (int)(4 * ceil((float)width / 4.0f)) * bytesPerPixel;
    int fileSize = paddedRowSize * height + HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&fileSize, 4, 1, outputFile);
    int reserved = 0x0000;
    fwrite(&reserved, 4, 1, outputFile);
    int dataOffset = HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&dataOffset, 4, 1, outputFile);
    int infoHeaderSize = INFO_HEADER_SIZE;
    fwrite(&infoHeaderSize, 4, 1, outputFile);
    fwrite(&width, 4, 1, outputFile);
    fwrite(&height, 4, 1, outputFile);
    short planes = 1; //always 1
    fwrite(&planes, 2, 1, outputFile);
    short bitsPerPixel = bytesPerPixel * 8;
    fwrite(&bitsPerPixel, 2, 1, outputFile);
    //write compression
    int compression = NO_COMPRESION;
    fwrite(&compression, 4, 1, outputFile);
    //write image size (in bytes)
    int imageSize = width * height * bytesPerPixel;
    fwrite(&imageSize, 4, 1, outputFile);
    int resolutionX = 11811; //300 dpi
    int resolutionY = 11811; //300 dpi
    fwrite(&resolutionX, 4, 1, outputFile);
    fwrite(&resolutionY, 4, 1, outputFile);
    int colorsUsed = MAX_NUMBER_OF_COLORS;
    fwrite(&colorsUsed, 4, 1, outputFile);
    int importantColors = ALL_COLORS_REQUIRED;
    fwrite(&importantColors, 4, 1, outputFile);
    int i = 0;
    int unpaddedRowSize = width * bytesPerPixel;

    for (i = 0; i < height; i++)
    {
        int pixelOffset = ((height - i) - 1) * unpaddedRowSize;
        fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
    }
    fclose(outputFile);
}