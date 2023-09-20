// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

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
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
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

    struct tm *fechaYTiempo;
    int tam = 128;
    char a[tam], m[tam], c[tam];
    fechaYTiempo = localtime(&stat.mtime);
    strftime(m, tam, "%Y-%m-%d %H:%M:%S", fechaYTiempo);
    fechaYTiempo = localtime(&stat.ctime);
    strftime(c, tam, "%Y-%m-%d %H:%M:%S", fechaYTiempo);
    fechaYTiempo = localtime(&stat.atime);
    strftime(a, tam, "%Y-%m-%d %H:%M:%S", fechaYTiempo);
    fprintf(stdout, "\nDATOS INODO %d:\ntipo=%c\npermisos=%d\natime: %s\nctime: %s\nmtime: %s\nnLinks=%d\ntamEnBytesLog=%d\nnumBloquesOcupados=%d\n", ninodo, stat.tipo, stat.permisos, a, c, m, stat.nlinks, stat.tamEnBytesLog, stat.numBloquesOcupados);

    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error desmontando el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}