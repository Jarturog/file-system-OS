// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Programa mi_link.c que crea un enlace a un fichero, llamando a la función mi_link() de la capa de directorios.
*/
int main(int argc, char const *argv[]){
    //Comprobación de que el número de argumentos es el correcto
    if(argc != 4){
        fprintf(stderr, ROJO "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n" RESET);
        return FALLO;
    }
    // Obtención del nombre del dispositivo
    const char *nombreDisco = argv[1];
    const char *rutaOriginal = argv[2];
    const char *rutaEnlace = argv[3];
    // si las rutas son directorios error
    if (rutaOriginal[strlen(rutaOriginal) - 1] == '/' || rutaEnlace[strlen(rutaEnlace) - 1] == '/') {
        fprintf(stderr, ROJO "Error, las rutas han de apuntar a ficheros, no a directorios\n" RESET);
        return FALLO;
    }
    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }
    // Llamada a mi_link
    int error = mi_link(rutaOriginal, rutaEnlace);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }
    // desmontar el dispositivo
    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}
