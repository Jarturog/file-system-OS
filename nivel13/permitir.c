// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

int main(int argc, char const *argv[]){
    if(argc != 4){
        fprintf(stderr, ROJO "Sintaxis: ./permitir <nombre_dispositivo><ninodo><permisos>\n" RESET);
        return FALLO;
    }
    
    const char *nombre = argv[1];
    char ninodo = atoi(argv[2]);
    unsigned char permisos = atoi(argv[3]);

    // Montaje del dispositivo virtual
    if (bmount(nombre) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }

    if(mi_chmod_f(ninodo, permisos) == FALLO){
        fprintf(stderr, ROJO "Error con mi_chmod_f en permitir.c\n" RESET);
        return FALLO;
    }

    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error desmontando el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}