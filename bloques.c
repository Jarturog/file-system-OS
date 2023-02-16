#include "bloques.h"
static int descriptor = 0;

int bmount(const *char camino){
    descriptor = open(camino,O_RDWR|O_CREAT,PERMISOS_RW); // si no existe se crea
    if(descriptor < 0){                                   //en cualquier caso se abre
        return FAILURE;
        fprintf(stderr,"Error")
    }
    return SUCCESS;
    
}

int bwrite(unsigned int nbloque, const void *buf){
    memset();
    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    write(descriptor, /*array de caracteres de 0s*/, BLOCKSIZE);
}

int bumount(){
    close(descriptor);
}

int bread(unsigned int nbloque, void *buf){
    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    read(descriptor, /*array de caracteres de 0s*/, BLOCKSIZE);
}