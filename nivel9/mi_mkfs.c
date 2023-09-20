// Autores: Marta González Juan y Juan Arturo Abaurrea Calafell 
#include "directorios.h"

int main(int argc, char **argv){

    //Comprobación de que el número de argumentos es el correcto
    if(argc != 3){
        fprintf(stderr, ROJO "Error de sintaxis. Uso correcto: ./mi_mkfs <nombre_dispositivo> <nbloques>\n" RESET);
        return FALLO;
    }

    //Obtención de los argumentos pasados por la línea de comandos
    char *nombre = argv[1];
    int nbloques = atoi(argv[2]);
    int ninodos = nbloques / 4; //Se calcula el número de inodos a partir del número de bloques, por heurística ninodos = nbloques/4

    //Comprobación de que el número de bloques es suficiente para el formato nivel 1
   if(nbloques < 4){
       fprintf(stderr, ROJO "Error: El número de bloques debe ser mayor o igual a 4\n" RESET);
       return FALLO;
    }

    //Montaje del dispositivo virtual
    if(bmount(nombre) == FALLO){
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    //Inicialización de los bloques del dispositivo virtual a 0s
    unsigned char buffer[BLOCKSIZE];
    if (memset(buffer, 0, BLOCKSIZE) == NULL){
        return FALLO;
    }
    
    for(int i = 0; i < nbloques; i++){
        if(bwrite(i, buffer) == FALLO){
            fprintf(stderr, ROJO "Error de escritura en el bloque %i\n", i);
            return FALLO;
        }
    }


    //Inicialización del superbloque
    if(initSB(nbloques, ninodos) == FALLO){
        fprintf(stderr, ROJO "Error de inicialización del superbloque\n" RESET);
        return FALLO;
    }


    //Inicialización del mapa de bits de bloques
    if(initMB() == FALLO){
        fprintf(stderr, ROJO "Error de inicialización del mapa de bits\n" RESET);
        return FALLO;
    }


    //Inicialización del array de inodos
    if(initAI() == FALLO){
        fprintf(stderr, ROJO "Error de inicialización del array de inodos\n" RESET);
        return FALLO;
    }

    if (reservar_inodo('d', 7) == FALLO) {
        fprintf(stderr, ROJO "Error al reservar inodo para el directorio raíz\n" RESET);
        return FALLO;
    }
 

    //Desmontaje del dispositivo virtual
    if(bumount() == FALLO){
        fprintf(stderr, ROJO "Error de desmontaje del dispositivo virtual\n" RESET);
        return FALLO;
    }
    
    return EXITO;
}
