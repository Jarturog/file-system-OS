// Autores: Marta González Juan y  Juan Arturo Abaurrea Calafell 

#include "directorios.h"
/**
*Ej. mover fichero: ./mi_mv disco   /dir2/dir22/fic221   /dir2/dir21/
*Ej. mover directorio: ./mi_mv disco   /dir3/   /dir1/
*Crea la entrada nombre (puede ser de fichero o directorio) en /destino/ y borra la entrada nombre en el directorio /origen/. 
*Hay que comprobar que exista /destino/ y que sea el nombre de un directorio (acabado en ‘/’).
*Hay que comprobar que no existe ya  /destino/nombre.
*Esta operación sólo afectará al nombre de la entrada correspondiente; el inodo del fichero o directorio movido sigue siendo el mismo.
*/
int main(int argc, char const *argv[]) {
    // Comprobar que se han pasado los argumentos necesarios
    if (argc != 4) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_mv <disco> </origen/nombre> </destino/>\n" RESET);
        return FALLO;
    }
    // Obtener los argumentos
    const char *nombreDisco = argv[1];
    const char *camino_origen = argv[2];
    const char *camino_destino = argv[3];
  

    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }
   
    int error = mi_mv(camino_origen, camino_destino);
    if(error < 0){
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error moviendo\n" RESET);
        } else {
            mostrar_error(error);
        }
    }

    // desmontar el dispositivo
    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}