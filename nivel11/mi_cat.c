// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Programa (comando) que muestra TODO el contenido de un fichero.
*/
int main(int argc, char const *argv[]){
    //Comprobación de que el número de argumentos es el correcto
    if(argc != 3){
        fprintf(stderr, ROJO "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }

    // Obtención del nombre del dispositivo
    const char *nombreDisco = argv[1];
    const char *ruta = argv[2];
    const unsigned int tambuffer = 1500;
    unsigned int offset = 0, bytes = 0;
    char buffer[tambuffer];
    memset(buffer, 0, tambuffer);
    // Si es un directorio error, no se puede leer en uno
    if ((ruta[strlen(ruta) - 1]) == '/') {
        fprintf(stderr, ROJO "No se puede leer en directorios.\n" RESET); 
        return FALLO;
    }
    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    int leidos = mi_read(ruta, buffer, offset, tambuffer);
    while (leidos > 0){ // fin cuando leidos = 0
        bytes += leidos;
        if(write(1, buffer, leidos) < 0){
            mostrar_error_buscar_entrada(bytes);
            return FALLO;
        }
        memset(buffer, 0, tambuffer);
        offset += tambuffer;
        leidos = mi_read(ruta, buffer, offset, tambuffer);
    }
    if (leidos < 0) {
        mostrar_error_buscar_entrada(leidos);
        bytes = 0;
    }
    // comprobar bytes == inodo.tamEnBytesLog == tamaño físico del fichero externo 
    fprintf(stderr, "\nTotal_leidos: %i\n", bytes);
    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}