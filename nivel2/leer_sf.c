#include <stdio.h>
#include "ficheros_basico.h"

int main(int argc, char **argv) {
    // Comprobación de que el número de argumentos es el correcto
    if (argc != 2) {
        fprintf(stderr,"Error de sintaxis. Uso correcto: ./leer_sf <nombre_dispositivo>\n");
        return FALLO;
    }

    // Obtención del nombre del dispositivo
    char *nombre = argv[1];

    // Montaje del dispositivo virtual
    
    if (bmount(nombre) == FALLO) {
        fprintf(stderr,"Error de montaje del dispositivo virtual\n");
        return FALLO;
    }

    // Lectura del superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr,"Error de lectura del superbloque\n");
        return FALLO;
    }

    // Mostramos la información almacenada en el superbloque
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n",  SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n",  SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
    printf("totBloques = %d\n", SB.totBloques);
    printf("totInodos = %d\n", SB.totInodos);
    printf("sizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));





// Iteramos sobre la lista enlazada de inodos libres

    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int contInodos = 0;
    int i, j;
    
    for (i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        bread(i, inodos);
        for (j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            if (contInodos < SB.totInodos) {
                if (inodos[j].tipo == 'l') {
                    printf("%d ", inodos[j].punterosDirectos[0]);
                    contInodos++;
                }
            } else {
               break;
            }
        }
        bwrite(i, inodos);
        if (contInodos >= SB.totInodos) {
            break;
        }
    }
    if (contInodos < SB.totInodos) {
        for (i = contInodos; i < SB.totInodos; i++) {
            printf("%d ", i);
        }
    }
    printf("-1\n");

    // Desmontaje del dispositivo virtual
    if (bumount() == FALLO) {
        fprintf(stderr, "Error de desmontaje del dispositivo virtual\n");
        return FALLO;
    }

    return EXITO;
}

