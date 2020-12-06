/**
 * Actualmente este fichero lo que hace es un ls
 */

#include "iostream" //Importante para poder imprimir por pantalla entre otras cosas
#include <fstream>  //Manejo de ficheros de entrada y salida
#include <cstring>  //Utilizado para comparar Strings
#include <dirent.h> //Manejo entre directorios
#include <cmath>
#include <chrono>

char origen[256];  //El path origen que me han pasado
char destino[256]; //El path origen que me han pasado
int op = -1;
int mGauss[5][5] = {{1, 4, 7, 4, 1},
                    {4, 16, 26, 16, 4},
                    {7, 26, 41, 26, 7},
                    {4, 16, 26, 16, 4},
                    {1, 4, 7, 4, 1}};

int mxSobel[3][3] = {{1, 2, 1},
                     {0, 0, 0},
                     {-1, -2, -1}};

int mySobel[3][3] = {{-1, 0, 1},
                     {-2, 0, 2},
                     {-1, 0, 1}};
typedef struct infoImagen
{
    char B = 'B';            // Array de chars "BM"
    char M = 'M';            // Array de chars "BM"
    int sFile;               // Tamaño del fichero
    int reservado = 0;       //Espacio reservado
    int offsetImagen = 54;   //Inicio del contenido de los pixeles de la imagen
    int sCabecera = 40;      // Tamaño de la cabecera
    int anchura;             // Anchura de la imagen
    int altura;              // Altura de la imagen
    short nPlanos = 1;       // Numero de planos de la imagen
    short bitPorPixel = 24;  //Bits por pixeles de la imange
    int compresion = 0;      // Compresion de la imagen
    int sImagen;             // Tamaño total solo de la imagen (altura*anchura*3)
    int rX = 2835;           // Resolucion horizontal
    int rY = 2835;           // Resolucion vertical
    int sColor = 0;          // Tamaño de la tabla de color
    int colorImportante = 0; // Colores Importantes
    unsigned char *imagen;   // Datos de la imange BMP
} infoImagen;

typedef struct tiempo
{
    int total = 0;
    long loadTime = 0;
    int gaussTime = 0;
    int sobelTime = 0;
    int storeTime = 0;

} tiempo;
using namespace std;

char *obtenerFilePath(char *path, char *fichero);
int operacion(char *fichero, tiempo *time);
infoImagen leerImagen(const char *fileName, short *error);
int escribirImagen(const char *filePathDestino, infoImagen imagen);
unsigned char *gauss(infoImagen datos);
unsigned char *sobel(infoImagen datos, unsigned char *imagen);
int comprobarBMP(infoImagen datos);
void printError(int tipo, char **argv)
{
    switch (tipo)
    {
    case 0:
        cout << "Wrong format:\n";
        break;
    case 1:
        cout << "Unexpected operation: " << argv[1] << "\n";
        break;
    case 2:
        cout << "Cannot open directory [" << argv[2] << "]\n";
        break;
    case 3:
        cout << "Output directory [" << argv[3] << "] does not exist\n";
        break;
    }
    cout << "  image-seq operation in_path out_path\n    operation: copy, gauss, sobel\n$\n";
}
int main(int argc, char *argv[])
{
    cout << "$image-seq ";
    //Si el número de argumentos que me pasa el programa es menor que 4 es que está mal
    if (argc != 4)
    {
        printError(0, argv);
        return -1;
    }
    cout << argv[1] << " " << argv[2] << " " << argv[3] << "\n";
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
        printError(1, argv);
        return -1;
    }
    memcpy(origen, argv[2], strlen(argv[2]));
    memcpy(destino, argv[3], strlen(argv[3]));
    if (origen[strlen(origen) - 1] != '/')
        strcat(origen, "/"); // En el caso de que no exista la barra en dir origen
    if (destino[strlen(destino) - 1] != '/')
        strcat(destino, "/"); // En el caso de que no exista la barra en dir origen
    cout << "Input path: " << origen << "\n"
         << "Output path: " << destino << "\n";
    struct dirent *eDirOrigen;          // Lee los ficheros que hay en el directorio de origen
    DIR *dirOrigen = opendir(origen);   // Obtengo todos los ficheros del origen
    DIR *dirDestino = opendir(destino); // Obtengo todos los ficheros del destino
    // Debe de existir los dos directorios
    if (dirOrigen == NULL)
    {
        printError(2, argv);
        return -1;
    }
    if (dirDestino == NULL)
    {
        printError(3, argv);
        return -1;
    }

    while ((eDirOrigen = readdir(dirOrigen)) != NULL) // Mientras el elemento que me pase el directorio no sea nulo
    {
        if (strcmp(eDirOrigen->d_name, ".") && strcmp(eDirOrigen->d_name, "..")) // Evito que utilicen como fichero el . y ..
        {
            tiempo time;
            auto start_time = chrono::high_resolution_clock::now();
            if (operacion(eDirOrigen->d_name, &time) == -1) // Tareas de la imagen
                return -1;
            auto end_time = chrono::high_resolution_clock::now();
            time.total = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
            cout << "File: \"" << origen << eDirOrigen->d_name << "\" (time: " << time.total << ")\n"
                 << "  Load time: " << time.loadTime << "\n";
            if (op == 2 || op == 3)
            {
                cout << "  Gauss time: " << time.gaussTime << "\n";
                if (op == 3)
                    cout << "  Sobel time: " << time.sobelTime << "\n";
            }
            cout << "  Store time: " << time.storeTime << "\n";
        }
    }
    closedir(dirOrigen);  //Cierro el directorio de origen
    closedir(dirDestino); //Cierro el directorio de destino
    cout << "$\n";
    return 0;
}

/* Esta función realiza la acción indicada por el usuario a cada uno de los archivos */
int operacion(char *fichero, tiempo *time)
{
    char *filePathOrigen = obtenerFilePath(origen, fichero);
    short error = 0;
    /*---------------- Leer Imagen -------------------*/
    auto start_time = chrono::high_resolution_clock::now();
    infoImagen imagenOrigen = leerImagen(filePathOrigen, &error);
    auto end_time = chrono::high_resolution_clock::now();
    time->loadTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
    free(filePathOrigen);
    if (error != 0)
        return -1;
    infoImagen imagenDestino;
    imagenDestino.altura = imagenOrigen.altura;
    imagenDestino.anchura = imagenOrigen.anchura;
    imagenDestino.sFile = imagenOrigen.sImagen + 54;
    imagenDestino.sImagen = imagenOrigen.sImagen;
    if (op == 2 || op == 3)
    {
        /*---------------- Gauss -------------------*/
        start_time = chrono::high_resolution_clock::now();
        unsigned char *imagenDestinoChar = gauss(imagenOrigen);
        end_time = chrono::high_resolution_clock::now();
        time->gaussTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        if (op == 3)
        {
            /*---------------- Sobel -------------------*/
            start_time = chrono::high_resolution_clock::now();
            imagenDestinoChar = sobel(imagenOrigen, imagenDestinoChar);
            end_time = chrono::high_resolution_clock::now();
            time->sobelTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        }
        imagenDestino.imagen = imagenDestinoChar;
    }
    else
    {
        imagenDestino.imagen = imagenOrigen.imagen;
    }

    /* Elegir la imagen que se va a Escribir */

    char *filePathDestino = obtenerFilePath(destino, fichero);

    /*---------------- Escribir el nuevo fichero -------------------*/
    start_time = chrono::high_resolution_clock::now();
    int errorEscribir = escribirImagen(filePathDestino, imagenDestino);
    end_time = chrono::high_resolution_clock::now();
    time->storeTime = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
    free(imagenOrigen.imagen); //Liberar la imagen leida y sobreescrita
    if (op > 1)
    {
        free(imagenDestino.imagen);
    }
    free(filePathDestino); //Liberar el path de destino
    if (errorEscribir == -1)
        return -1;
    return 0;
}

/* Esta funcion obtiene el path donde se encuentra el fichero juntando la carpeta origen y el nombre del archivo */
char *obtenerFilePath(char *path, char *fichero)
{
    char *filePath = (char *)malloc(256); // Creo un espacio donde guardar los paths a los archivos
    strcat(filePath, path);               // Copio la carpeta
    strcat(filePath, fichero);            // Copio el nombre del fichero
    return filePath;                      // Devuelvo el puntero al path completo hacia el archivo
}

/* Esta función lee la imagen que ha recibido por parámetro y comprueba que todos los parámetros necesarios 
   son correctos. También actualiza los valores anchura y altura pasados por parámetro*/
infoImagen leerImagen(const char *fileName, short *error)
{
    FILE *leerDF = fopen(fileName, "rb"); // Descriptor de fichero de la imagen
    infoImagen tmp;
    if (leerDF == NULL)
    {
        perror("Error al intentar leer el fichero");
        *error = -1;
        return tmp;
    }
    if ((fread(&tmp, 1, 2, leerDF)) == 0)
    {
        perror("Error de escritura");
        *error = -1;
        return tmp;
    }
    //fseek(leerDF, 2, 0);
    if ((fread(&tmp.sFile, sizeof(int), 6, leerDF)) == 0)
    {
        perror("Error de escritura");
        *error = -1;
        return tmp;
    }
    if ((fread(&tmp.nPlanos, sizeof(short), 2, leerDF)) == 0)
    {
        perror("Error de escritura");
        *error = -1;
        return tmp;
    }
    if ((fread(&tmp.compresion, sizeof(int), 6, leerDF)) == 0)
    {
        perror("Error de escritura");
        *error = -1;
        return tmp;
    }
    if (comprobarBMP(tmp) != 0)
    {
        *error = 1;
        return tmp;
    }
    int unpaddedRowSize = tmp.anchura * 3;
    int paddedRowSize;
    if (unpaddedRowSize % 4 != 0)
        paddedRowSize = unpaddedRowSize + (4 - (unpaddedRowSize % 4));
    else
        paddedRowSize = unpaddedRowSize;

    int totalSize = unpaddedRowSize * tmp.altura;
    tmp.imagen = (unsigned char *)malloc(totalSize);

    unsigned char *currentRowPointer = tmp.imagen + ((tmp.altura - 1) * unpaddedRowSize);
    for (int i = 0; i < tmp.altura; i++)
    {
        fseek(leerDF, tmp.offsetImagen + (i * paddedRowSize), SEEK_SET);
        if ((fread(currentRowPointer, 1, unpaddedRowSize, leerDF)) == 0)
        {
            perror("Error de escritura");
            *error = -1;
            return tmp;
        }
        currentRowPointer -= unpaddedRowSize;
    }
    fclose(leerDF); // Cierro el descriptor de fichero
    return tmp;
}

/*Esta funcion obtiene todos los parametros necesarios para leer una imagen y escribe la imagen pasada por argumento
   ademas del lugar donde guardarla, la altura y la anchura*/
int escribirImagen(const char *fileName, infoImagen imagen)
{
    FILE *escribirDF = fopen(fileName, "wb");
    if (escribirDF == NULL)
    {
        perror("Error al intentar crear el archivo de destino");
        return -1;
    }
    // Escribir cada uno de los parámetros de la cabecera
    if (fwrite(&imagen, 1, 2, escribirDF) == 0)
    { // Escribo BM
        perror("Error de escritura al escribir los datos de la imagen");
        return -1;
    }
    if (fwrite(&imagen.sFile, sizeof(int), 6, escribirDF) == 0)
    { // Escribo los siguientes enteros de la cabecera
        perror("Error de escritura al escribir los datos de la imagen");
        return -1;
    }
    if (fwrite(&imagen.nPlanos, sizeof(short), 2, escribirDF) == 0)
    { // Escribo los shorts de la cabecera
        perror("Error de escritura al escribir los datos de la imagen");
        return -1;
    }
    if (fwrite(&imagen.compresion, sizeof(int), 6, escribirDF) == 0)
    { // Escribo los últimos enteros de la cabecera
        perror("Error de escritura al escribir los datos de la imagen");
        return -1;
    }
    fseek(escribirDF, imagen.offsetImagen, SEEK_SET); // Establezco la posición donde se escribe la imagen
    int unpaddedRowSize = imagen.anchura * 3;
    int paddedRowSize;
    if (unpaddedRowSize % 4 != 0)
        paddedRowSize = unpaddedRowSize + (4 - (unpaddedRowSize % 4));
    else
        paddedRowSize = unpaddedRowSize;
    for (int i = 0; i < imagen.altura; i++)
    {
        int pixelOffset = ((imagen.altura - i) - 1) * unpaddedRowSize;
        if (fwrite(&imagen.imagen[pixelOffset], 1, paddedRowSize, escribirDF) == 0)
        {
            perror("Error de escritura al escribir los pixeles");
            return -1;
        }
    }
    fclose(escribirDF); // Cierro el descriptor de fichero de escribir
    return 0;
}

int comprobarBMP(infoImagen datos)
{

    if (datos.B != 'B' || datos.M != 'M')
    {
        perror("El archivo no es BMP");
        return -1;
    }
    else if (datos.nPlanos != 1)
    {
        perror("El número de planos es distinto de 1");
        return -1;
    }
    else if (datos.bitPorPixel != 24)
    {
        perror("El número de bits por pixel no es 24");
        return -1;
    }
    else if (datos.compresion != 0)
    {
        perror("La compresion del archivo no es 0");
        return -1;
    }
    return 0;
}

unsigned char *sobel(infoImagen datos, unsigned char *imagen)
{
    int width = datos.anchura;
    int height = datos.altura;
    int w = 8;
    unsigned char *pixels = imagen;
    int linea = width * 3;
    int tmp[6];
    int size = height * linea;
    unsigned char *pixelsN = (unsigned char *)malloc(size);
    for (int i = 0; i <= height - 1; i += 1)
        for (int j = 0; j <= linea - 1; j += 3)
        {
            {
                tmp[0] = 0;
                tmp[1] = 0;
                tmp[2] = 0;
                tmp[3] = 0;
                tmp[4] = 0;
                tmp[5] = 0;
                for (int s = -1; s <= 1; s++)
                {
                    for (int t = -1; t <= 1; t++)
                    {
                        int byte = (i + s) * linea + j + t * 3;
                        for (short color = 0; color <= 4; color+=2)
                        {
                            if (byte >= 0 && byte <= size - 1 && 0 <= j + t * 3 && t * 3 + j <= linea - 1)
                            {
                                tmp[color] += mxSobel[s + 1][t + 1] * pixels[byte];
                                tmp[color+1] += mySobel[s + 1][t + 1] * pixels[byte];
                            }
                            byte += 1;
                        }
                    }
                }
                pixelsN[(i * linea) + j] = (unsigned char) ((abs(tmp[0]) + abs(tmp[1]))/ w);
                pixelsN[(i * linea) + j + 1] = (unsigned char)((abs(tmp[2]) + abs(tmp[3]))/ w);
                pixelsN[(i * linea) + j + 2] = (unsigned char)((abs(tmp[4]) + abs(tmp[5]))/ w);
            }
        }
    free(pixels);
    return pixelsN;
}

unsigned char *gauss(infoImagen datos)
{
    int width = datos.anchura;
    int height = datos.altura;
    unsigned char *pixels = datos.imagen;
    int w = 273;
    int linea = width * 3;
    int tmp[3];
    int size = height * linea;
    unsigned char *pixelsN = (unsigned char *)malloc(size);
    size -= 1;
    int columnaByte;
    int columnaGauss;
    int filaGauss;
    height -= 1;
    for (int i = 0; i <= height; i += 1)
        for (int j = 0; j <= linea - 1; j += 3)
        {
            {
                tmp[0] = 0;
                tmp[1] = 0;
                tmp[2] = 0;
                for (int s = -2; s <= 2; s++)
                {
                    for (int t = -2; t <= 2; t++)
                    {
                        columnaByte = j + t * 3;
                        filaGauss = s + 2;
                        columnaGauss = t + 2;
                        int byte = (i + s) * linea + columnaByte;
                        for (short color = 0; color <= 2; color++)
                        {
                            if (byte >= 0 && byte <= size && 0 <= columnaByte && columnaByte <= linea - 1)
                                tmp[color] += mGauss[filaGauss][columnaGauss] * pixels[byte];
                            byte += 1;
                            columnaByte += 1;
                        }
                    }
                }
                tmp[0] /= w;
                tmp[1] /= w;
                tmp[2] /= w;
                pixelsN[(i * linea) + j] = (unsigned char)(tmp[0]);
                pixelsN[(i * linea) + j + 1] = (unsigned char)(tmp[1]);
                pixelsN[(i * linea) + j + 2] = (unsigned char)(tmp[2]);
            }
        }
    return pixelsN;
}
