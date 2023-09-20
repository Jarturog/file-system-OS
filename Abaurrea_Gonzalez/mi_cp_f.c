// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan

#include "directorios.h"

/**
 * Copiar un fichero en otro directorio: 
 * 
 * Ej: /mi_cp_f disco   /dir1/dir12/fic121     /dir3/
 * Tras la copia del contenido, el fichero nuevo ha de tener los mismos permisos que el original.
 * Los bloques ocupados del fichero en /destino/ han de ser los mismos que los del fichero en origen.
*/
int main(int argc, char const *argv[]) {
    // Comprobar que se han pasado los argumentos necesarios
    if (argc != 4) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_cp_f <disco> </origen/nombre> </destino/>\n" RESET);
        return FALLO;
    }
    // Obtener los argumentos
    const char *nombreDisco = argv[1];
    const char *caminoOrigen = argv[2];
    const char *destino = argv[3];

    // comprobación de que sea un fichero
    if (caminoOrigen[strlen(caminoOrigen) - 1] == '/') {
        fprintf(stderr, ROJO "Error, no se puede copiar un directorio con el comando de copiar un fichero\n" RESET);
        return FALLO;
    }
    

    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }
    int error = mi_cp(caminoOrigen, destino);
    if(error < 0){
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error copiando el fichero\n" RESET);
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