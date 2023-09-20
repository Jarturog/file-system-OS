#include "bloques.h"
int main(int argc, char **argv){

    //Comprobación de que el número de argumentos es el correcto
    if(argc != 3){
        printf("Uso: ./mi_mkfs <nombre_dispositivo> <nbloques>\n");
        return FALLO;
    }

    //Obtención de los argumentos pasados por la línea de comandos
    char *nombre = argv[1];
    int nbloques = atoi(argv[2]);

    //Montaje del dispositivo virtual
    if(bmount(nombre) == FALLO){
        printf("Error de montaje del dispositivo virtual\n");
        return FALLO;
    }

    //Inicialización de los bloques del dispositivo virtual a 0s
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE);
    for(int i = 0; i < nbloques; i++){
        if(bwrite(i, buffer) == -1){
            printf("Error de escritura en el bloque %i\n", i);
            return FALLO;
        }
    }

    //Desmontaje del dispositivo virtual
    if(bumount() == FALLO){
        printf("Error de desmontaje del dispositivo virtual\n");
        return FALLO;
    }

    return EXITO;
}