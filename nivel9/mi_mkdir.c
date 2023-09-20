// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

int main(int argc, char const *argv[]) {
    // Comprobar argumentos
    if (argc != 4) {
        fprintf(stderr, ROJO "Sintaxis: %s <disco> <permisos> </ruta>\n" RESET, argv[0]);
        return FALLO;
    }

    // Obtener argumentos
    const char *nombreDisco = argv[1];
    const char *camino = argv[3];
    const unsigned char permisos = atoi(argv[2]);

    // si mala sintaxis o es un fichero
    if (camino[strlen(camino)-1] != '/') {
        fprintf(stderr, ROJO "Error al usar mi_mkdir para crear un fichero en vez de un directorio.\nPara crear un fichero se ha de usar mi_touch\nPara indicar que se quiere crear un directorio se ha de indicar una '/' al final\n" RESET);
        return FALLO;
    }

    // Comprobar permisos
    if (permisos > 7) {
        fprintf(stderr, ROJO "Error: modo inválido<<%i>>.\n" RESET, permisos);
        return FALLO;
    }

    // montar disco
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    // Crear directorio
    int error = mi_creat(camino, permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    // desmontar disco
    if (bumount() == FALLO) {
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}