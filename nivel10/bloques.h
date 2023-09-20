// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include <stdio.h>  //printf(), fprintf(), stderr, stdout, stdin
#include <fcntl.h> //O_WRONLY, O_CREAT, O_TRUNC
#include <sys/stat.h> //S_IRUSR, S_IWUSR
#include <sys/time.h> //gettimeofday()
#include <stdlib.h>  //exit(), EXIT_SUCCESS, EXIT_FAILURE, atoi()
#include <unistd.h> // SEEK_SET, read(), write(), open(), close(), lseek()
#include <errno.h>  //errno
#include <string.h> // strerror()

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
#define DEBUGTRADBI 0 // debug de traducir_bloques_inodo
#define DEBUGBUSENT 0 // debug de buscar_entrada
#define CACHELRUNOTFIFO 0 // tipo de caché para entrada y escritura: 1 si LRU, 0 si FIFO
#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 0
#define DEBUGN5 0
#define DEBUGN6 0
#define DEBUGN7 0
#define DEBUGN8 0
#define DEBUGN9 0
#define DEBUGN10 1

int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);
