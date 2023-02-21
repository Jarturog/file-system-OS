#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define ROJO "\x1b[31m"
#define RESET "\x1b[0m"
#define FAILURE -1
#define SUCCESS 0
#define BLOCKSIZE 1024 // 1K
#define PERMISOS_RW 0666 // permisos de leer y escribir en octal

// declaraciones
int bmount(const *char camino);
int bwrite(unsigned int nbloque, const void *buf);
int bumount();
int bread(unsigned int nbloque, void *buf);