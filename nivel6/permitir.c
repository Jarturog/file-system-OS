// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "ficheros.h"

int main(int argc, char const *argv[]){
    if(argc != 4){
        fprintf(stderr, ROJO "Error de sintaxis. Uso correcto: ./permitir <nombre_dispositivo><ninodo><permisos>\n" RESET);
        return FALLO;
    }
    
    const char *nombre = argv[1];
    char ninodo = atoi(argv[2]);
    unsigned char permisos = atoi(argv[3]);

    // Montaje del dispositivo virtual
    if (bmount(nombre) == FALLO) {
        fprintf(stderr,"Error de montaje del dispositivo virtual\n");
        return FALLO;
    }

    if(mi_chmod_f(ninodo, permisos) == FALLO){
        fprintf(stderr, ROJO "Error con mi_chmod_f en permitir.c\n" RESET);
        return FALLO;
    }

    if (bumount() == FALLO){
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        return FALLO;
    }
    return EXITO;
}