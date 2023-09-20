// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan

#include "directorios.h"
/**
 * Ej. renombrar fichero: ./mi_rn disco /dir2/dir22/fic221 fic222
 * Ej. renombrar directorio: ./mi_rn disco /dir2/dir23/  dir24
 * Busca antiguo en su directorio superior (dentro de /ruta) y le cambia el nombre por nuevo.
 * antiguo y nuevo han de ser del mismo tipo, ambos fichero o directorio (acabado en / en tal caso).
 * Hay que comprobar que no existe ya  /ruta/nuevo.
 * Esta operación sólo afectará al nombre de la entrada correspondiente; el inodo del fichero o directorio sigue siendo el mismo.
*/
int main(int argc, char const *argv[]) {
    // Comprobar que se han pasado los argumentos necesarios
    if (argc != 4) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo>\n" RESET);
        return FALLO;
    }
    // Obtener los argumentos
    const char *nombreDisco = argv[1];
    const char *camino = argv[2];
    const char *nombre_nuevo = argv[3];

    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }   

    int error = 0;
    if ((error = mi_rn(camino, nombre_nuevo)) < 0){
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error renombrando\n" RESET);
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