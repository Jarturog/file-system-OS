// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Programa (comando) que muestra la información acerca del inodo de un fichero o directorio,
 * llamando a la función mi_stat() de la capa de directorios, que a su vez llamará a mi_stat_f() de la capa de ficheros.
*/
int main(int argc, char const *argv[]) {
    // Comprobar argumentos
    if (argc != 3) {
        fprintf(stderr, ROJO "Sintaxis: ./mi_stat <disco> </ruta>\n" RESET);
        return FALLO;
    }

    // Obtener argumentos
    char *camino = (char*)argv[2];
    char *nombreDisco = (char*)argv[1];
    struct STAT p_stat;
    
    if (bmount(nombreDisco) == FALLO) {
        mostrar_error_buscar_entrada(FALLO);
        return FALLO;
    }
    int p_inodo = mi_stat(camino, &p_stat);
    if (p_inodo < 0) {
        mostrar_error_buscar_entrada(p_inodo);
        return FALLO;
    }
    
    struct tm *fechaYTiempo;
    int tam = 128;
    char a[tam], m[tam], c[tam];
    fechaYTiempo = localtime(&p_stat.mtime);
    strftime(m, tam, "%Y-%m-%d %H:%M:%S", fechaYTiempo);
    fechaYTiempo = localtime(&p_stat.ctime);
    strftime(c, tam, "%Y-%m-%d %H:%M:%S", fechaYTiempo);
    fechaYTiempo = localtime(&p_stat.atime);
    strftime(a, tam, "%Y-%m-%d %H:%M:%S", fechaYTiempo);
    fprintf(stdout, "Nº de inodo: %d\ntipo: %c\npermisos: %d\natime: %s\nctime: %s\nmtime: %s\nnlinks: %d\ntamEnBytesLog: %d\nnumBloquesOcupados: %d\n", p_inodo, p_stat.tipo, p_stat.permisos, a, c, m, p_stat.nlinks, p_stat.tamEnBytesLog, p_stat.numBloquesOcupados);

    if (bumount() ==  FALLO) {
        mostrar_error_buscar_entrada(FALLO);
        return FALLO;
    }
    
    return EXITO; 
}