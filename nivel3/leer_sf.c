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

    //Probación el tamaño del tipo time_t para vuestra plataforma/compilador:
    //printf("\nsizeof time_t is: %ld\n", sizeof(time_t));

    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");
    int reservado = reservar_bloque(); // Actualiza el SB
    bread(posSB, &SB);                 // Actualizar los valores del SB

    printf("Se ha reservado el bloque físico nº %i que era el 1º libre indicado por el MB.\n", reservado);
    printf("SB.cantBloquesLibres: %i\n", SB.cantBloquesLibres);
    liberar_bloque(reservado);
    bread(posSB, &SB); // Actualizar los valores del SB

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
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    struct inodo inodo;
    int ninodo = 0; //el directorio raiz es el inodo 0
    leer_inodo(ninodo, &inodo);
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %i\n", inodo.permisos);
    printf("ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n", ninodo, atime, mtime, ctime);
    printf("nlinks: %i\n", inodo.nlinks);
    printf("tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
    printf("Número de bloques ocupados: %i\n", inodo.numBloquesOcupados);

    //Liberación
    if (bumount() == FALLO)
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        return FALLO;
    }
    return EXITO;
}

