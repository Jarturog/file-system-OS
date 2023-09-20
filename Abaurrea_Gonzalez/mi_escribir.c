// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Permite escribir texto en una posición de un fichero (offset).
*/
int main(int argc, char const *argv[]){
    //Comprobación de que el número de argumentos es el correcto
    if(argc != 5){
        fprintf(stderr, ROJO "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

    // Obtención del nombre del dispositivo
    const char *nombreDisco = argv[1];
    const char *ruta = argv[2];
    const char *texto = argv[3];
    const unsigned int offset = atoi(argv[4]);
    const unsigned int longitud = strlen(texto);

    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }

    // Si es un directorio error, no se puede escribir en uno
    if ((ruta[strlen(ruta) - 1]) == '/') {
        fprintf(stderr, ROJO "No se puede escribir en directorios.\n" RESET);
        return FALLO;
    }

    fprintf(stdout, "Longitud texto: %i\n", longitud);
    int bytes = mi_write(ruta, texto, offset, longitud);
    if (bytes < 0) {
        mostrar_error_buscar_entrada(bytes);
        bytes = 0;
    }
    fprintf(stdout, "Bytes escritos: %i\n", bytes);

    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error desmontando el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}
