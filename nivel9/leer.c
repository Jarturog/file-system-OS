// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

#define tambuffer 1500

int main(int argc, char const *argv[]){

    //Comprobación de que el número de argumentos es el correcto
    if(argc != 3){
        fprintf(stderr, ROJO "Error de sintaxis. Uso correcto: ./leer <nombre_dispositivo> <ninodo>\n" RESET);
        return FALLO;
    }

    const char *nombre = argv[1];
    char ninodo = atoi(argv[2]);
    unsigned int offset = 0;
    unsigned int bytes = 0;
    char buffer[tambuffer];
    memset(buffer, 0, tambuffer);

    // Montaje del dispositivo virtual
    if (bmount(nombre) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    int leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    while (leidos > 0){ // fin cuando leidos = 0
        bytes += leidos;
        if(write(1, buffer, leidos) == FALLO){
            fprintf(stderr, ROJO "Error en leer.c\n" RESET);
            return FALLO;
        }
        memset(buffer, 0, tambuffer);
        offset += tambuffer;
        leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    }

    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) == FALLO){
        fprintf(stderr, ROJO "Error creando stat en leer.c\n" RESET);
        return FALLO;
    }
    fprintf(stderr, "total_bytesleidos: %u\ntamEnBytesLog: %u\n", bytes, stat.tamEnBytesLog);

    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}
