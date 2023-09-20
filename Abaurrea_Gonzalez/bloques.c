// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor = 0, inside_sc = 0;
static sem_t *mutex;
#if MMAP
static void *ptrSFM; //static void *ptrSFM: puntero a memoria compartida
static int tamSFM; //static int tamSFM: tamaño memoria compartida
#endif
/**
 * Función para montar el dispositivo virtual, y dado que se trata de un fichero, esa acción consistirá en abrirlo.
 * camino: array de char's pasados por parámetro a open(). Es el nombre del fichero.
 * Devuelve el descriptor que se le ha sido asignado al fichero.
 * 
 * Internamente llama a umask() y open().
*/
int bmount(const char *camino) {
    if (descriptor > 0) {
        close(descriptor);
    }
    if (!mutex) { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem(); 
        if (mutex == SEM_FAILED) {
            return FALLO;
        }
    }
    umask(000); // establecer permisos de lectura/escritura para todos los usuarios
    descriptor = open(camino, O_RDWR | O_CREAT, 0666); // 0666 todos los permisos menos los de ejecución
    if (descriptor < 0) { // si error
        fprintf(stderr, ROJO "Error al abrir el archivo\n" RESET);
        return FALLO;
    }
#if MMAP
    struct stat st;
    fstat(descriptor, &st);
    tamSFM = st.st_size; 
    ptrSFM = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
    if (ptrSFM == (void *)-1) {
        fprintf(stderr, ROJO "Fallo utilizando MMAP. Error %d: %s\n", errno, strerror(errno)); 
    }
#endif
    return descriptor;
}

/**
 * Desmonta el dispositivo virtual, llama a la función close() para liberar el descriptor de fichero.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a close().
*/
int bumount() {
#if MMAP
    if (msync(ptrSFM, tamSFM, MS_SYNC) < 0) {
        fprintf(stderr, ROJO "Fallo volcando las páginas de memoria a disco. Error %d: %s\n", errno, strerror(errno)); 
    }
    if (munmap(ptrSFM, tamSFM) < 0) {
        fprintf(stderr, ROJO "Fallo liberando memoria. Error %d: %s\n", errno, strerror(errno)); 
    }
#endif
    descriptor = close(descriptor);
    deleteSem();
    if (descriptor < 0) {
        fprintf(stderr, ROJO "Error al cerrar el archivo\n" RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Escribe 1 bloque en el dispositivo virtual, en el bloque físico especificado por nbloque.
 * nbloque: número del bloque a escribir.
 * buf: contenido del tamaño de BLOCKSIZE a escribir en el dispositivo virtual.
 * Devuelve el número de bytes escritos o FALLO si ha habido un error.
 * 
 * Internamente llama a write() y lseek().
*/
int bwrite(unsigned int nbloque, const void *buf) {
    int res;
    off_t desplazamiento = nbloque * BLOCKSIZE;
#if MMAP
    res = BLOCKSIZE;
    if (desplazamiento + BLOCKSIZE > tamSFM) {
        res -= desplazamiento;
    }
    if (res > 0) {
        memcpy(ptrSFM + desplazamiento, buf, res);
    } else {
        fprintf(stderr, ROJO "Error al ser un bloque fuera del rango. No se ha realizado escritura.\n" RESET);
        return FALLO;
    }
#else
    if (lseek(descriptor, desplazamiento, SEEK_SET) < 0) {
        fprintf(stderr, ROJO "bwrite(): Error al establecer la posición del puntero.\n" RESET);
        return FALLO;
    }
    res = write(descriptor, buf, BLOCKSIZE);
    if (res == FALLO) {
        fprintf(stderr, ROJO "bwrite(): Error al escribir en el archivo.\n" RESET);
        return FALLO;
    }
#endif
    return res;
}

/**
 * Lee 1 bloque del dispositivo virtual, que se corresponde con el bloque físico especificado por nbloque
 * nbloque: número del bloque a leer.
 * buf: buffer donde se va a guardar el bloque leído.
 * Devuelve el número de bytes leídos o FALLO si ha habido un error.
 * 
 * Internamente llama a read() y lseek().
*/
int bread(unsigned int nbloque, void * buf) {
    int nbytes_leidos;
    off_t desplazamiento = nbloque * BLOCKSIZE;
#if MMAP
    nbytes_leidos = BLOCKSIZE;
    if (desplazamiento + BLOCKSIZE > tamSFM) {
        nbytes_leidos -= desplazamiento;
    }
    if (nbytes_leidos > 0) {
        memcpy(buf, ptrSFM + desplazamiento, nbytes_leidos);
    } else {
        fprintf(stderr, ROJO "Error al ser un bloque fuera del rango. No se ha realizado lectura.\n" RESET);
        return FALLO;
    }
#else
    if (lseek(descriptor, desplazamiento, SEEK_SET) < 0) { // mueve el puntero del fichero en el offset correcto
        fprintf(stderr, ROJO "bread(): Error al establecer la posición del puntero\n" RESET);
        return FALLO;
    }
    nbytes_leidos = read(descriptor, buf, BLOCKSIZE); // lee los nbytes (BLOCKSIZE) contenidos a partir de la posición correspondiente al nº de bloque especificado
    if (nbytes_leidos < 0 || nbytes_leidos != BLOCKSIZE) { // comprueba si se ha producido algún error
        fprintf(stderr, ROJO "bread(): Error al leer en el archivo\n" RESET);
        return FALLO;
    }
#endif
    return nbytes_leidos;
}

/**
 * Hace esperar a un hilo en caso de que haya otro en la sección crítica.
 * 
 * Internamente llama a waitSem().
*/
void mi_waitSem() {
    if (!inside_sc) { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
}

/**
 * Indica que el hilo ha salido de la sección crítica, por lo que otro puede entrar.
 * 
 * Internamente llama a signalSem().
*/
void mi_signalSem() {
    inside_sc--;
    if (!inside_sc) {
        signalSem(mutex);
    }
}
