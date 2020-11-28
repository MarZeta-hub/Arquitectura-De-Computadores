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

typedef struct infoImagen
{
    char *BM;   // Array de chars "BM"
    int sFile;  // Tamaño del fichero
    int reservado;  //Espacio reservado
    int offsetImagen; //Inicio del contenido de los pixeles de la imagen
    int sCabecera; // Tamaño de la cabecera
    int anchura; // Anchura de la imagen
    int altura; // Altura de la imagen
    short nPlanos; // Numero de planos de la imagen
    short bitPorPixel; //Bits por pixeles de la imange
    int compresion; // Compresion de la imagen
    int sImagen; // Tamaño total solo de la imagen (altura*anchura*3)
    int rX; // Resolucion horizontal
    int rY; // Resolucion vertical
    int sColor; // Tamaño de la tabla de color
    int colorImportante; // Colores Importantes
    unsigned char *imagen; // Datos de la imange BMP
} infoImagen;

using namespace std;

char *obtenerFilePath(char *path, char *fichero);
int operacion(char *fichero);
infoImagen leerImagen(const char *fileName);
void escribirImagen(const char *filePathDestino, infoImagen imagen);
infoImagen gauss(infoImagen imagen);
infoImagen sobel(infoImagen datos);

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
        op = 1; //En el caso de que el trabajo sea copy
    }
    else if (strcmp(argv[1], "gauss") == 0)
    {
        op = 2; //En el caso de que el trabajo sea gauss
    }
    else if (strcmp(argv[1], "sobel") == 0)
    {
        op = 3; //En el caso de que el trabajo sea sobel
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
    closedir(dirOrigen);    //Cierro el directorio de origen
    closedir(dirDestino);    //Cierro el directorio de destino
    return 0;
}

/* Esta función realiza la acción indicada por el usuario a cada uno de los archivos */
int operacion(char *fichero)
{
    char *filePathOrigen = obtenerFilePath(origen, fichero);
    infoImagen imagenOrigen = leerImagen(filePathOrigen);
    free(filePathOrigen);
    infoImagen imagenDestino;
    switch (op)
    {
    case 1:
        imagenDestino = imagenOrigen;
        break;
    case 2:
        imagenDestino = gauss(imagenOrigen);
        break;
    case 3:
        imagenDestino = sobel(imagenOrigen);
        break;
    default:
        perror("El programa nunca debería llegar aqui");
        return -1;
    }
    char *filePathDestino = obtenerFilePath(destino, fichero);
    escribirImagen(filePathDestino , imagenDestino);
    free(imagenOrigen.imagen);
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
infoImagen leerImagen(const char *fileName)
{
    FILE *leerDF = fopen(fileName, "rb"); // Descriptor de fichero de la imagen
    infoImagen tmp;
    fread(&tmp, 1, 2, leerDF);
    fread(&tmp.sFile, sizeof(int), 6, leerDF);
    fread(&tmp.nPlanos, sizeof(short), 2, leerDF);
    fread(&tmp.compresion, sizeof(int), 6, leerDF);
    tmp.imagen = (unsigned char *)malloc(tmp.sImagen);
    fseek(leerDF,tmp.offsetImagen, SEEK_SET);
    fread(tmp.imagen, tmp.sImagen, 1, leerDF);
    fclose(leerDF); // Cierro el descriptor de fichero
    return tmp;
}

/*Esta funcion obtiene todos los parametros necesarios para leer una imagen y escribe la imagen pasada por argumento
   ademas del lugar donde guardarla, la altura y la anchura*/
void escribirImagen(const char *fileName, infoImagen imagen)
{
    FILE *escribirDF = fopen(fileName, "wb");
    // Escribir cada uno de los parámetros de la cabecera
    fwrite(&imagen, 1, 2, escribirDF); // Escribo BM 
    fwrite(&imagen.sFile, sizeof(int), 6, escribirDF); //Escribo los siguientes enteros de la cabecera
    fwrite(&imagen.nPlanos, sizeof(short), 2, escribirDF); // Escribo los shorts de la cabecera
    fwrite(&imagen.compresion, sizeof(int), 6, escribirDF); //Escribo los últimos enteros de la cabecera
    fseek(escribirDF,imagen.offsetImagen, SEEK_SET); // Establezco la posición donde se escribe la imagen
    fwrite(imagen.imagen, imagen.sImagen, 1, escribirDF); // Escribo la imagen
    fclose(escribirDF); // Cierro el descriptor de fichero de escribir
}

infoImagen gauss(infoImagen datos)
{
    cout << "utilizando Gauss valor de anchura y tal" << datos.anchura << "\n";

    return datos;
}

infoImagen sobel(infoImagen datos)
{
    cout << "utilizando Sobel valor de anchura y tal" << datos.anchura << "\n";
    return datos;
}