// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "ficheros.h"

#define tambuffer 1500

int main(int argc, char const *argv[]){

    //Comprobación de que el número de argumentos es el correcto
    if(argc != 3){
        fprintf(stderr, ROJO "Error de sintaxis. Uso correcto: ./leer <nombre_dispositivo> <ninodo>\n");
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
        fprintf(stderr,"Error de montaje del dispositivo virtual\n");
        return FALLO;
    }

    int leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    while (leidos > 0){ // fin cuando leidos = 0
        bytes += leidos;
        if(write(1, buffer, leidos) == FALLO){
            fprintf(stderr, "Error en leer.c\n");
            return FALLO;
        }
        memset(buffer, 0, tambuffer);
        offset += tambuffer;
        leidos = mi_read_f(ninodo, buffer, offset, tambuffer);
    }

    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) == FALLO){
        fprintf(stderr, "Error creando stat en leer.c\n");
        return FALLO;
    }
    fprintf(stderr, "total_bytesleidos: %u\ntamEnBytesLog: %u\n", bytes, stat.tamEnBytesLog);

    if (bumount() == FALLO){
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        return FALLO;
    }
    return EXITO;
}
