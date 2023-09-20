// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Cambia los permisos de un fichero o directorio, llamando a la función mi_chmod() de la capa de directorios.
 * Los permisos se indican en octal, será 4 para sólo lectura (r--), 2 para sólo escritura (-w-), 1 para sólo ejecución (--x).
*/
int main(int argc, char const *argv[]){

    //Comprobación de que el número de argumentos es el correcto
    if(argc != 4){
        fprintf(stderr, ROJO "Error de sintaxis. Uso correcto: ./mi_chmod <disco> <permisos> </ruta>\n" RESET);
        return FALLO;
    }

    const char *nombreDisco = argv[1];
    const unsigned char permisos = atoi(argv[2]);
    const char *ruta = argv[3];

    // Comprobar que los permisos son válidos
    if (permisos > 7){
        fprintf(stderr, ROJO "Error: permisos han de estar entre 0 y 7 incluidos.\n" RESET);
        return FALLO;
    }

    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    int error = mi_chmod(ruta, permisos);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}
