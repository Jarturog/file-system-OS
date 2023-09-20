#include "bloques.h"

static int descriptor = 0;

int bmount(const char * camino) {
    umask(000); // establecer permisos de lectura/escritura para todos los usuarios
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor < 0) {
        fprintf(stderr, ROJO "Error al abrir el archivo\n" RESET);
        return FALLO;
    }
    return descriptor;
}

int bumount() {
    int res = close(descriptor);
    if (res < 0) {
        fprintf(stderr, ROJO "Error al cerrar el archivo\n" RESET);
        return FALLO;
    }
    return EXITO;
}

int bwrite(unsigned int nbloque, const void * buf) {
    off_t desplazamiento;
    desplazamiento = nbloque * BLOCKSIZE;
    if (lseek(descriptor, desplazamiento, SEEK_SET) < 0) {
        fprintf(stderr, ROJO "Error al establecer la posición del puntero\n" RESET);
        return FALLO;
    }
    int res = write(descriptor, buf, BLOCKSIZE);
    if (res == FALLO) {
        fprintf(stderr, ROJO "Error al escribir en el archivo\n" RESET);
        return FALLO;
    }
    return res;
}

int bread(unsigned int nbloque, void * buf) {
    off_t desplazamiento = nbloque * BLOCKSIZE;
    if (lseek(descriptor, desplazamiento, SEEK_SET) < 0) { // mueve el puntero del fichero en el offset correcto
        fprintf(stderr, ROJO "Error al establecer la posición del puntero\n" RESET);
        return FALLO;
    }
    ssize_t nbytes_leidos = read(descriptor, buf, BLOCKSIZE); // lee los nbytes (BLOCKSIZE) contenidos a partir de la posición correspondiente al nº de bloque especificado
    if (nbytes_leidos < 0 || nbytes_leidos != BLOCKSIZE) { // comprueba si se ha producido algún error
        fprintf(stderr, ROJO "Error al leer en el archivo\n" RESET);
        return FALLO;
    }
    return nbytes_leidos;
}