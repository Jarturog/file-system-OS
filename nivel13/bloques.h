// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <sys/time.h> //gettimeofday()
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()
#include <sys/mman.h> // mmap

#define CACHELRUNOTFIFO 1 // tipo de caché para entrada y escritura: 1 si LRU, 0 si FIFO
#define MMAP 1 // 1 si MMAP activado, 0 si no
#define GRIS "\x1b[94m"
#define ROJO "\x1b[31m"
#define VERDE "\x1b[32m"
#define AMARILLO "\x1b[33m"
#define AZUL "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CIAN "\x1b[36m"
#define RESET "\x1b[0m"
#define BLOCKSIZE 1024 // bytes
#define EXITO 0 //para gestión errores
#define FALLO -1 //para gestión errores
#define DEBUGTRADBI 1 // debug de traducir_bloques_inodo
#define DEBUGBUSENT 1 // debug de buscar_entrada
#define DEBUGSIMUL 1 // debug de la simulación
#define DEBUGN2 1
#define DEBUGN3 1
#define DEBUGN4 1
#define DEBUGN5 1
#define DEBUGN6 1
#define DEBUGN7 1
#define DEBUGN8 1
#define DEBUGN9 1
#define DEBUGN10 1
#define DEBUGN12 1
#define DEBUGN13 1

// nivel 1
int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);
// nivel 11
void mi_signalSem();
void mi_waitSem();
