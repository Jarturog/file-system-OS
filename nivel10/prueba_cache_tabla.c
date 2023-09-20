#include "directorios.h"


int main(int argc,char const *argv[]){

   if (argc!=3) {
      fprintf(stderr, ROJO "Sintaxis: ./prueba_cache_tabla <nombre_dispositivo> <texto>\n" RESET);
      exit(FALLO);
   }
    // Montar dispositivo virtual
    if(bmount(argv[1])<0){
         return FALLO;
    }

    //obtenemos el texto y su longitud
    char buffer_texto[strlen(argv[2])];
    memset(buffer_texto, 0, strlen(argv[2]));
    strcpy(buffer_texto, argv[2]);
    int nbytes=strlen(buffer_texto);
    int offset=0;
    char rutas[5][6]={"/fic1", "/fic2", "/fic3", "/fic4", "/fic5"};
    char orden_acceso[12][6] = {"/fic2", "/fic3", "/fic2", "/fic1", "/fic5", "/fic2", "/fic4", "/fic5", "/fic3", "/fic2", "/fic5", "/fic2"};

    // Crear ficheros
    for (int i=0; i<5; i++){
        mi_creat(rutas[i], 6);
        fprintf(stderr, MAGENTA "[prueba cache ()→ creado %s]\n" RESET, rutas[i]);  
    }
    //Escrituras
    for (int j=0; j<12; j++){
        fprintf(stderr, CIAN "\n[prueba cache ()→ acceso a %s]\n" RESET, orden_acceso[j] );
        mi_write(orden_acceso[j], buffer_texto, offset, nbytes);
    }  

    // Desmontar dispositivo virtual
    if(bumount()<0){
        return FALLO;
    }
    return EXITO;
}