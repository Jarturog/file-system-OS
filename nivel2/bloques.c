 #include "bloques.h"

static int descriptor = 0;


int bmount(const char *camino)
{
  umask(000); // establecer permisos de lectura/escritura para todos los usuarios
  descriptor =  open(camino, O_RDWR | O_CREAT, 0666);
  if(descriptor == FALLO){
   
    perror("Error al abrir el archivo");
    return FALLO;
  
  }

  return descriptor;
}
int bumount() {
    int res;
    
    res = close(descriptor);
    if (res == FALLO) {
        perror("Error al cerrar el archivo");
        return FALLO;
    }
    
    return EXITO;
}
int bwrite(unsigned int nbloque, const void *buf) {
    off_t desplazamiento;
    int res;
    
    desplazamiento = nbloque * BLOCKSIZE;
    res = lseek(descriptor, desplazamiento, SEEK_SET);
    if (res == FALLO) {
        perror("Error al establecer la posición del puntero");
        return FALLO;
    }
    
    res = write(descriptor, buf, BLOCKSIZE);
    if (res == FALLO) {
        perror("Error al escribir en el archivo");
        return FALLO;
    }
    
    return res;
}
int bread(unsigned int nbloque, void *buf){
     off_t desplazamiento = nbloque * BLOCKSIZE;
    if (lseek(descriptor, desplazamiento, SEEK_SET) == -1) { // mueve el puntero del fichero en el offset correcto
        return FALLO;
    }
    ssize_t nbytes_leidos = read(descriptor, buf, BLOCKSIZE); // lee los nbytes (BLOCKSIZE) contenidos a partir de la posición correspondiente al nº de bloque especificado
    if (nbytes_leidos == -1 || nbytes_leidos != BLOCKSIZE) { // comprueba si se ha producido algún error
        return FALLO;
    }
    return BLOCKSIZE;
}



