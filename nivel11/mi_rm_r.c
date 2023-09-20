// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Borra todo el contenido de un directorio no vacío
 * (similar al comando rm con la opción -r de Linux)
*/
int main(int argc, char const *argv[]){
    //Comprobación de que el número de argumentos es el correcto
    if(argc != 3){
        fprintf(stderr, ROJO "Sintaxis: ./mi_rm_r <disco> </ruta>\n" RESET);
        return FALLO;
    }
    // Obtención del nombre del dispositivo
    const char *nombreDisco = argv[1];
    const char *ruta = argv[2];
    // si la rutas es un directorio entonces error
    if (ruta[strlen(ruta) - 1] != '/') {
        fprintf(stderr, ROJO "Error, la ruta ha de apuntar a un fichero, no a un directorio\n" RESET);
        return FALLO;
    }
    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }
    int error = mi_rm_r(ruta);
    if(error < 0){
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error borrando recursivamente\n" RESET);
        } else {
            mostrar_error_buscar_entrada(error);
        }
    }
    // desmontar el dispositivo
    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}
