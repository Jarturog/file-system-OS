// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Crea un fichero, llamando a la función mi_creat().
*/
int main(int argc, char const *argv[]) {
    // Comprobar argumentos
    if (argc != 4) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_touch <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }
    // Obtener argumentos
    const char *nombreDisco = argv[1];
    const char *camino = argv[3];
    const unsigned char permisos = atoi(argv[2]);

    // si mala sintaxis o es un directorio
    if (camino[strlen(camino)-1] == '/') {
        fprintf(stderr, ROJO "Error al usar mi_touch para crear un directorio en vez de un fichero.\nPara crear un directorio se ha de usar mi_mkdir\nPara indicar que se quiere crear un fichero no se ha de indicar una '/' al final\n" RESET);
        return FALLO;
    }

    // Comprobar permisos
    if (permisos > 7) {
        fprintf(stderr, ROJO "Error: modo inválido<<%i>>.\n" RESET, permisos);
        return FALLO;
    }

    // montar disco
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }

    // Crear fichero
    int error = mi_creat(camino, permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    // desmontar disco
    if (bumount() == FALLO) {
        fprintf(stderr, ROJO "Error desmontando el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}