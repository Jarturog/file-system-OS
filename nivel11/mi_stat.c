// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"

/**
 * Programa (comando) que muestra la información acerca del inodo de un fichero o directorio,
 * llamando a la función mi_stat() de la capa de directorios, que a su vez llamará a mi_stat_f() de la capa de ficheros.
*/
int main(int argc, char const *argv[]) {
    // Comprobar argumentos
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: %s <disco> </ruta>\n", argv[0]);
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
    if(p_inodo < 0) {
        mostrar_error_buscar_entrada(p_inodo);
        return FALLO;
    }
    
    struct tm *aux;
    char atime[100], mtime[100], ctime[100];

    aux = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", aux);
    aux = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", aux);
    aux = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", aux);

    fprintf(stdout, "Nº de inodo: %d\n", p_inodo);
    fprintf(stdout, "tipo: %c\n", p_stat.tipo);
    fprintf(stdout, "permisos: %d\n", p_stat.permisos);
    fprintf(stdout, "atime: %s\n", atime);
    fprintf(stdout, "ctime: %s\n", ctime);
    fprintf(stdout, "mtime: %s\n", mtime);
    fprintf(stdout, "nlinks: %d\n", p_stat.nlinks);
    fprintf(stdout, "tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    fprintf(stdout, "numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);

    if (bumount() ==  FALLO) {
        mostrar_error_buscar_entrada(FALLO);
        return FALLO;
    }
    
    return EXITO; 
}