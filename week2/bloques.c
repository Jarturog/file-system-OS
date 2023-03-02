#include "bloques.h"

static int descriptor = 0;

/**
 * Monta el dispositivo virtual y al ser un fichero lo abre.
 *
 * Parámetros de entrada:
 * - camino: dirección de memoria del dispositivo virtual
 * 
 * Devuelve FAILURE si ha habido un error y SUCCESS en caso contrario
 */
int bmount(const *char camino)
{
    // umask(000) // en caso de que no vaya bien por motivos de sistema operativo descomentar esta línea
    descriptor = open(camino, O_RDWR | O_CREAT, PERMISOS_RW); // si no existe se crea
    if (descriptor < 0) // en cualquier caso se abre
    { 
        fprintf(stderr, "Error\n");
        return FAILURE;
    }
    return SUCCESS;
}

/**
 * Desonta el dispositivo virtual.
 *
 * Devuelve FAILURE si ha habido un error y SUCCESS en caso contrario
 */
int bumount()
{
    if (close(descriptor) < 0) // si error
    { 
        fprintf(stderr, "Error");
        return FAILURE;
    }
    return SUCCESS;
}

/**
 * Escribe un bloque en el dispositivo virtual,
 * en el bloque físico especificado por nbloque.
 *
 * Parámetros de entrada:
 * - nbloque: el número del bloque en el cual se va a escribir
 * - buf: puntero de lo que se va a escribir
 * 
 * Devuelve FAILURE si ha habido un error y la cantidad de bytes
 * escritos en caso contrario (el valor esperado es de BLOCKSIZE)
 */
int bwrite(unsigned int nbloque, const void *buf)
{
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) < 0) // si error
    {
        fprintf(stderr, "Error\n");
        return FAILURE;
    }

    int bytes = write(descriptor, buf, BLOCKSIZE);

    if (bytes < 0) // si error
    {
        fprintf(stderr, "Error\n");
        return FAILURE;
    }
    return bytes;
}

/**
 * Lee un bloque del dispositivo virtual,
 * que se corresponde con el bloque físico especificado por nbloque.
 *
 * Parámetros de entrada:
 * - nbloque: el número del bloque del cual se va a leer
 * - buf: puntero en el que se guardará lo leído
 * 
 * Devuelve FAILURE si ha habido un error y la cantidad de bytes
 * leídos en caso contrario (el valor esperado es de BLOCKSIZE)
 */
int bread(unsigned int nbloque, void *buf)
{
    if (lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET) < 0) // si error
    {
        fprintf(stderr, "Error\n");
        return FAILURE;
    }

    int bytes = read(descriptor, buf, BLOCKSIZE);

    if (bytes < 0) // si error
    {
        fprintf(stderr, "Error\n");
        return FAILURE;
    }
    return bytes;
}