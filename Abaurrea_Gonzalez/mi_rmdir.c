// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Programa mi_rm.c que borra un directorio, llamando a la función mi_unlink() de la capa de directorios.
*/
int main(int argc, char const *argv[]){
    //Comprobación de que el número de argumentos es el correcto
    if(argc != 3){
        fprintf(stderr, ROJO "Sintaxis: ./mi_rmdir <disco> </ruta>\n" RESET);
        return FALLO;
    }
    // Obtención del nombre del dispositivo
    const char *nombreDisco = argv[1];
    const char *ruta = argv[2];
    // si la rutas es un fichero entonces error
    if (ruta[strlen(ruta) - 1] != '/') {
        fprintf(stderr, ROJO "Error, la ruta ha de apuntar a un directorio, no a un fichero\n" RESET);
        return FALLO;
    }
    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }
    // Llamada a mi_unlink
    int error = mi_unlink(ruta);
    if (error < 0) {
        if (error == ERROR_DIRECTOR_NO_VACIO) {
            fprintf(stderr, ROJO "Error: El directorio %s no está vacío\n" RESET, ruta);
        } else {
            mostrar_error_buscar_entrada(error);
        }
    }
    // desmontar el dispositivo
    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error desmontando el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}
