// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "ficheros.h"

int main(int argc, char const *argv[]){

    unsigned int offsets[5] = {9000, 209000, 30725000, 409605000, 480000000};

    //Comprobación de que el número de argumentos es el correcto
    if (argc != 4){
        fprintf(stderr, ROJO "Error de sintaxis. Uso correcto: ./escribir <nombre_dispositivo><\"$(cat fichero)\"> <diferentes_inodos>\n"
        "Offsets: %u, %u, %u, %u, %u\n"
        "Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n" RESET, offsets[0], offsets[1], offsets[2], offsets[3], offsets[4]);
        return FALLO;
    }

    // Obtención del nombre del dispositivo
    const char *nombre = argv[1];
    const char *texto = argv[2];
    unsigned int diferentes_inodos = atoi(argv[3]);

    // Montaje del dispositivo virtual
    if (bmount(nombre) == FALLO) {
        fprintf(stderr,"Error de montaje del dispositivo virtual\n");
        return FALLO;
    }

    // Reserva de inodo
    unsigned int ninodo = reservar_inodo('f', 6);
    if (ninodo == FALLO)
    {
        fprintf(stderr, "Error reservando inodo en escribir.c\n");
        return FALLO;
    }

    unsigned int longitud = strlen(texto);
    fprintf(stdout, "longitud texto: %u\n\n", longitud);

    for (int i = 0; i < (sizeof(offsets) / sizeof(offsets[0])); i++){
        fprintf(stdout, "Nº inodo reservado: %u\n", ninodo);
        fprintf(stdout, "offset: %u\n", offsets[i]);
        unsigned int bytes = mi_write_f(ninodo, texto, offsets[i], longitud);
        if (bytes == FALLO){
            fprintf(stderr, "Error usando mi_write_f en escribir.c\n");
            return FALLO;
        }
        fprintf(stdout, "Bytes escritos: %d\n", bytes);
        char *buf = malloc(longitud);
        if (buf == NULL){
            fprintf(stderr, "Error en escribir.c");
            return FALLO;
        }
        if (memset(buf, 0, longitud) == NULL){
            fprintf(stderr, "Error en escribir.c");
            return FALLO;
        }

        struct STAT p_stat;
        // Obtencion de la información del inodo escrito
        if (mi_stat_f(ninodo, &p_stat) == FALLO){
            fprintf(stderr, "Error en mi_stat_f en escribir.c\n");
            return FALLO;
        }

        fprintf(stdout, "stat.tamEnBytesLog = %u\n", p_stat.tamEnBytesLog);
        fprintf(stdout, "stat.numBloquesOcupados = %u\n\n", p_stat.numBloquesOcupados);

        // si diferentes_indodos es 1
        if (diferentes_inodos == 1) { // sí se reservan diferentes inodos
            ninodo = reservar_inodo('f', 6); // reserva nuevo inodo
            if (ninodo == FALLO){
                fprintf(stderr, "Error al reservar inodo en escribir.c\n");
                return FALLO;
            }
        }
    }

    if (bumount() == FALLO){
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        return FALLO;
    }
    return EXITO;
}
