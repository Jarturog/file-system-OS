// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "ficheros.h"

int main(int argc, char const *argv[]){
    if(argc != 4){
        fprintf(stderr, ROJO "Sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n" RESET);
        return FALLO;
    }
    
    const char *nombre = argv[1];
    char ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    // Montaje del dispositivo virtual
    if (bmount(nombre) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    if (nbytes == 0){
        if(liberar_inodo(ninodo) == FALLO){
            fprintf(stderr, ROJO "Error liberando inodo en truncar.c\n" RESET);
            return FALLO;
        }
    }else{
        if(mi_truncar_f(ninodo, nbytes) == FALLO){
            fprintf(stderr, ROJO "Error llamando mi_truncar_f en truncar.c\n" RESET);
            return FALLO;
        }
    }

    struct STAT stat;
    if (mi_stat_f(ninodo, &stat) == FALLO){
        fprintf(stderr, ROJO "Error llamando mi_stat_f en truncar.c\n" RESET);
        return FALLO;
    }

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime(&stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    fprintf(stdout, "\nDATOS INODO %d:\n", ninodo);
    fprintf(stdout, "tipo=%c\n", stat.tipo);
    fprintf(stdout, "permisos=%d\n", stat.permisos);
    fprintf(stdout, "atime: %s\n", atime);
    fprintf(stdout, "ctime: %s\n", ctime);
    fprintf(stdout, "mtime: %s\n", mtime);
    fprintf(stdout, "nLinks=%d\n", stat.nlinks);
    fprintf(stdout, "tamEnBytesLog=%d\n", stat.tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados=%d\n", stat.numBloquesOcupados);

    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}