#include <stdio.h>
#include "ficheros_basico.h"

#define DEBUGN2 0
#define DEBUGN3 0
#define DEBUGN4 1

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

#if DEBUGN2
    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contlibres = 0;
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        if (bread(i, inodos) == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            if ((inodos[j].tipo == 'l')) {
                contlibres++;
                if (contlibres < 20) {
                    printf("%d ", contlibres);
                } else if (contlibres == 21) {
                    printf("... ");
                } else if ((contlibres > 24990) && (contlibres < SB.totInodos)) {
                    printf("%d ", contlibres);
                } else if (contlibres == SB.totInodos) {
                    printf("-1 \n");
                }
                contlibres--;
            }
            contlibres++;
        }
    }
#endif

#if DEBUGN3
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");
    int bloqueReservado = reservar_bloque(); // Actualiza el SB
    bread(posSB, & SB); // Actualizar los valores del SB
    printf("Se ha bloqueReservado el bloque físico nº %i que era el 1º libre indicado por el MB.\n", bloqueReservado);
    printf("SB.cantBloquesLibres: %i\n", SB.cantBloquesLibres);
    liberar_bloque(bloqueReservado);
    bread(posSB, & SB); // Actualizar los valores del SB
    printf("Liberamos ese bloque, y después SB.cantBloquesLibres: %i\n\n", SB.cantBloquesLibres);
    printf("MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    int bit = leer_bit(posSB);
    printf("leer_bit(%i) = %i\n", posSB, bit);
    bit = leer_bit(SB.posPrimerBloqueMB);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueMB, bit);
    bit = leer_bit(SB.posUltimoBloqueMB);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueMB, bit);
    bit = leer_bit(SB.posPrimerBloqueAI);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueAI, bit);
    bit = leer_bit(SB.posUltimoBloqueAI);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueAI, bit);
    bit = leer_bit(SB.posPrimerBloqueDatos);
    printf("leer_bit(%i) = %i\n", SB.posPrimerBloqueDatos, bit);
    bit = leer_bit(SB.posUltimoBloqueDatos);
    printf("leer_bit(%i) = %i\n", SB.posUltimoBloqueDatos, bit);
    printf("\nDATOS DEL DIRECTORIO RAIZ\n\n");
    struct tm * ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    struct inodo inodo;
    int ninodo = 0; //el directorio raiz es el inodo 0
    leer_inodo(ninodo, & inodo);
    ts = localtime( & inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime( & inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime( & inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n", ninodo, atime, mtime, ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
    intf("Número de bloques ocupados: %i\n", inodo.numBloquesOcupados);
#endif

#if DEBUGN4
    int nInodo = reservar_inodo('f', 6);
    if(nInodo == FALLO){
        fprintf(stderr, ROJO "Error reservando inodo" RESET);
        return FALLO;
    }
    struct inodo inodo;
    if(leer_inodo(nInodo, &inodo) == FALLO) { //Leemos el Inodo reservado
        fprintf(stderr, ROJO "Error leyendo inodo %i" RESET, nInodo);
        return FALLO;
    }
    if(bread(posSB, &SB) == FALLO){
        fprintf(stderr, ROJO "Error leyendo superbloque" RESET);
        return FALLO;
    }
    printf("\nINODO %d - TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n", nInodo);
    traducir_bloque_inodo(&inodo,8,1);
    traducir_bloque_inodo(&inodo,204,1);
    traducir_bloque_inodo(&inodo,30004,1);
    traducir_bloque_inodo(&inodo,400004,1);
    traducir_bloque_inodo(&inodo,468750,1);
    printf("\nDATOS DEL INODO RESERVADO: %d\n", nInodo);
    struct tm * ts;
    char atime[80];
    char mtime[80];
    char ctime[80];
    ts = localtime( & inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime( &inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime( &inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("ATIME: %s \nMTIME: %s \nCTIME: %s\n", atime, mtime, ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
    printf("Número de bloques ocupados: %i\n", inodo.numBloquesOcupados);
    printf("SB.posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
#endif

    //Liberación
    if (bumount() == FALLO)
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        return FALLO;
    }
    return EXITO;
}

