// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "ficheros.h"

/**
 * Escribe el contenido procedente de un buffer de memoria, buf_original, de tamaño nbytes, en un fichero/directorio.
 *  Le indicamos la posición de escritura inicial en bytes lógicos, offset, con respecto al inodo, y el número de bytes, nbytes, que hay que escribir.
 * ninodo: número del inodo donde se va a escribir la información de buf_original.
 * buf_original: buffer que contiene la información a escribir.
 * offset: byte desde el que se va a empezar a escribir.
 * nbytes: cantidad de bytes a escribir.
 * Devuelve la cantidad de bytes escritos o FALLO en caso de error.
 * 
 * Internamente llama a leer_inodo(), escribir_inodo(), traducir_bloque_inodo(), bread(), bwrite(), memcpy() y time().
*/
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    if ((inodo.permisos & 2) != 2) { // no hay permisos para escribir
        fprintf(stderr, ROJO "mi_write_f: Error, no se puede escribir si no se tienen permisos de escritura\n" RESET);
        return FALLO;
    }
    unsigned int bytesEscritos = 0, bytesEscritosAhora = 0;
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    unsigned int bloqueFisico = traducir_bloque_inodo(&inodo, primerBL, 1);
    if (bloqueFisico == FALLO) { 
        fprintf(stderr, ROJO "mi_write_f: Error pasando bloque lógico a físico\n" RESET);
        return FALLO;
    }
    char unsigned bufBloque[BLOCKSIZE];
    if (bread(bloqueFisico, bufBloque) == FALLO) {
        fprintf(stderr, ROJO "mi_write_f: Error leyendo\n" RESET);
        return FALLO;
    }
    unsigned int bytesAEscribir;
    if (primerBL == ultimoBL) {
        bytesAEscribir = nbytes;
    }else{ // primerBL < ultimoBL
        bytesAEscribir = BLOCKSIZE - desp1;
    }

    if (memcpy(bufBloque + desp1, buf_original, bytesAEscribir) == NULL) {
        fprintf(stderr, ROJO "mi_write_f: Error\n" RESET);
        return FALLO;
    }
 
    bytesEscritosAhora = bwrite(bloqueFisico, bufBloque);
    if (bytesEscritosAhora == FALLO) {
        fprintf(stderr, ROJO "mi_write_f: Error escribiendo bloques\n" RESET);
        return FALLO;
    }

    if (primerBL == ultimoBL) {
        bytesEscritos += nbytes; // desp2 - desp1 + 1 ?????
    }else{ // primerBL < ultimoBL
        bytesEscritos += bytesEscritosAhora - desp1;
    }
    
    if (primerBL < ultimoBL) {
        for (int bloque = 1 + primerBL; bloque < ultimoBL; bloque++) {
            bloqueFisico = traducir_bloque_inodo(&inodo, bloque, 1);
            if (bloqueFisico == FALLO) {
                fprintf(stderr, ROJO "mi_write_f: Error traduciendo bloque inodo\n" RESET);
                return FALLO;
            }
            bytesEscritosAhora = bwrite(bloqueFisico, buf_original + (BLOCKSIZE - desp1) + (bloque - primerBL - 1) * BLOCKSIZE);
            if (bytesEscritosAhora == FALLO)
            {
                fprintf(stderr, ROJO "mi_write_f: Error escribiendo bloques\n" RESET);
                return FALLO;
            }
            bytesEscritos += bytesEscritosAhora; // BLOCKSIZE
        }
        // último bloque
        bloqueFisico = traducir_bloque_inodo(&inodo, ultimoBL, 1);
        if (bloqueFisico == FALLO)
        {
            fprintf(stderr, ROJO "mi_write_f: Error traduciendo el último bloque del inodo\n" RESET);
            return FALLO;
        }
        if (bread(bloqueFisico, bufBloque) == FALLO)
        {
            fprintf(stderr, ROJO "mi_write_f: Error leyendo último bloque\n" RESET);
            return FALLO;
        }
        if (memcpy(bufBloque, buf_original + (nbytes - desp2 - 1), desp2 + 1) == NULL) {
            fprintf(stderr, ROJO "mi_write_f: Error\n" RESET);
            return FALLO;
        }
        bytesEscritosAhora = bwrite(bloqueFisico, bufBloque);
        if (bytesEscritosAhora == FALLO) {
            fprintf(stderr, ROJO "mi_write_f: Error escribiendo último bloque\n" RESET);
            return FALLO;
        }
        bytesEscritos += desp2 + 1;
    }
    if ((offset + nbytes) > inodo.tamEnBytesLog) { // si hemos escrito más allá del final del fichero (bytesEscritos > inodo.tamEnBytesLog) ???
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO) { // escribir inodo actualizado
        fprintf(stderr, ROJO "Error escribiendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    return bytesEscritos;
}

/**
 * Lee información de un fichero/directorio y la almacena en un buffer de memoria, buf_original.
 *  Le indicamos la posición de lectura inicial offset con respecto al inodo (en bytes) y el número de bytes nbytes que hay que leer.
 * ninodo: número del inodo del cual se va a leer.
 * buf_original: buffer en el que se almacenará lo leído.
 * offset: byte desde el que se va a empezar a leer.
 * nbytes: cantidad de bytes a leer.
 * Devuelve la cantidad de bytes leídos o FALLO en caso de error.
 * 
 * Internamente llama a leer_inodo(), escribir_inodo(), traducir_bloque_inodo(), bread(), memcpy() y time().
*/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
    unsigned int bytesLeidos = 0;
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    if ((inodo.permisos & 4) != 4) { // no hay permisos para leer
        fprintf(stderr, ROJO "mi_read_f: Error, no se puede leer si no se tienen permisos de lectura\n" RESET);
        return FALLO;
    }
    // comprobación del EOF
    if (offset >= inodo.tamEnBytesLog) {
        return bytesLeidos;
    }
    if ((offset + nbytes) >= inodo.tamEnBytesLog) { 
        nbytes = inodo.tamEnBytesLog - offset;
    }
    char unsigned bufBloque[BLOCKSIZE];
    unsigned int primerBL = offset / BLOCKSIZE;
    unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;
    unsigned int desp1 = offset % BLOCKSIZE;
    unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
    int bloqueFisico = traducir_bloque_inodo(&inodo, primerBL, 0);
    unsigned int bytesALeer;
    if (primerBL == ultimoBL) {
        bytesALeer = nbytes;
    } else { // primerBL < ultimoBL
        bytesALeer = BLOCKSIZE - desp1;
    }

    if (bloqueFisico != FALLO) {
        if (bread(bloqueFisico, bufBloque) == FALLO) {
            fprintf(stderr, ROJO "mi_read_f: Error escribiendo bloques\n" RESET);
            return FALLO;
        }
        if (memcpy(buf_original, bufBloque + desp1, bytesALeer) == NULL) {
            fprintf(stderr, ROJO "mi_read_f: Error\n" RESET);
            return FALLO;
        }
    }

    bytesLeidos = bytesALeer; 

    if (primerBL < ultimoBL) {
        //bytesLeidos += BLOCKSIZE - desp1; 
        for (int bloque = 1 + primerBL; bloque < ultimoBL; bloque++) {
            bloqueFisico = traducir_bloque_inodo(&inodo, bloque, 0);
            bytesLeidos += BLOCKSIZE;
            if (bloqueFisico == FALLO) {
                continue;
            }
            if (bread(bloqueFisico, bufBloque) == FALLO) {
                fprintf(stderr, ROJO "mi_read_f: Error escribiendo bloques\n" RESET);
                return FALLO;
            }
            if ((memcpy(buf_original + (BLOCKSIZE - desp1) + (bloque - primerBL - 1) * BLOCKSIZE, bufBloque, BLOCKSIZE)) == NULL) {
                fprintf(stderr, ROJO "mi_read_f: Error haciend memcpy\n" RESET);
                return FALLO;
            }
        }
        // último bloque
        bloqueFisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);
        if (bloqueFisico != FALLO) {
            if (bread(bloqueFisico, bufBloque) == FALLO) {
                fprintf(stderr, ROJO "mi_read_f: Error leyendo último bloque\n" RESET);
                return FALLO;
            }
            if (memcpy(buf_original + (nbytes - desp2 - 1), bufBloque, desp2 + 1) == NULL) {
                fprintf(stderr, ROJO "mi_read_f: Error\n" RESET);
                return FALLO;
            }
        }
        bytesLeidos += desp2 + 1;
    }

    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    if ((offset + nbytes) > inodo.tamEnBytesLog) { // si hemos escrito más allá del final del fichero (bytesLeidos > inodo.tamEnBytesLog) ???
        inodo.tamEnBytesLog = nbytes + offset;
        inodo.ctime = time(NULL);
    }

    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO) { // escribir inodo actualizado
        fprintf(stderr, ROJO "Error escribiendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    return bytesLeidos;
}

/**
 * Devuelve la metainformación de un fichero/directorio: 
 *  tipo, permisos, cantidad de enlaces de entradas en directorio, tamaño en bytes lógicos, timestamps y cantidad de bloques ocupados en la zona de datos.
 * ninodo: número del inodo del cual se va a crear un STAT.
 * p_stat: puntero en el que se va a guardar el STAT creado.
 * Devuelve FALLO o EXITO dependiendo de si ha habido un error o no.
 * 
 * Internamente llama a leer_inodo().
*/
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "mi_stat_f: Error leyendo inodo\n" RESET);
        return FALLO;
    }
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXITO;
}

/**
 * Cambia los permisos de un fichero/directorio con el valor que indique el argumento permisos.
 * ninodo: número del inodo del cual se va a cambiar los permisos.
 * permisos: nuevos permisos del inodo indicado por ninodo.
 * Devuelve FALLO o EXITO dependiendo de si ha habido un error o no.
 * 
 * Internamente llama a leer_inodo(), escribir_inodo() y time().
*/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos) {
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "mi_chmod_f: Error leyendo inodo\n" RESET);
        return FALLO;
    }
    inodo.ctime = time(NULL);
    inodo.permisos = permisos;
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "mi_chmod_f: Error actualizando inodo\n" RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Trunca un fichero/directorio a los bytes indicados como nbytes, liberando los bloques necesarios.
 * ninodo: número del inodo del cual se liberarán los bloques.
 * nbytes: número de bytes a los que se va a truncar el inodo.
 * Devuelve la cantidad de bloques liberados o FALLO si ha habido un error.
 * 
 * Internamente llama a leer_inodo(), escribir_inodo(), time() y liberar_bloques_inodo().
*/
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
    // Lectura del inodo
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    // Hay que comprobar que el inodo tenga permisos de escritura.
    if ((inodo.permisos & 2) != 2) {
        fprintf(stderr, ROJO "mi_truncar_f: Error, no se puede truncar si no se tienen permisos de escritura\n" RESET);
        return FALLO;
    }
    // No se puede truncar más allá del tamaño en bytes lógicos (EOF) del fichero/directorio.
    if (nbytes > inodo.tamEnBytesLog) {
        fprintf(stderr, ROJO "mi_truncar_f: Error, no se puede truncar más allá del EOF\n" RESET);
        return FALLO;
    }
    // Para saber que nº de bloque lógico le hemos de pasar como primer bloque lógico a liberar:
    unsigned int primerBL = nbytes/BLOCKSIZE;
    if (nbytes % BLOCKSIZE != 0) { // un bloque más si no encaja exacto
        primerBL++;        
    }
    // Truncamos
    int bloquesLiberados = liberar_bloques_inodo(primerBL, &inodo);
    if (bloquesLiberados == FALLO) {
        fprintf(stderr, ROJO "mi_truncar_f: Error liberando bloques del inodo %i\n" RESET, ninodo);
        return FALLO;
    }
    // Actualizar mtime, ctime, tamEnBytesLog y numBloquesOcupados.
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= bloquesLiberados;
    // Escritura del inodo
    if (escribir_inodo(ninodo, &inodo) == FALLO) {
        fprintf(stderr, ROJO "Error escribiendo inodo %i" RESET, ninodo);
        return FALLO;
    }
    // Devolver la cantidad de bloques liberados.
    return bloquesLiberados;
}