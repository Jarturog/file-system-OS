// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"
void mostrar_buscar_entrada(char *camino, char reservar);
void mostrar_buscar_entrada(char *camino, char reservar) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, & p_inodo_dir, & p_inodo, & p_entrada, reservar, 6)) < 0) {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
    return;
}

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
    fprintf(stdout,"DATOS DEL SUPERBLOQUE\n");
    fprintf(stdout,"posPrimerBloqueMB = %d\n",  SB.posPrimerBloqueMB);
    fprintf(stdout,"posUltimoBloqueMB = %d\n",  SB.posUltimoBloqueMB);
    fprintf(stdout,"posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    fprintf(stdout,"posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    fprintf(stdout,"posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    fprintf(stdout,"posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    fprintf(stdout,"posInodoRaiz = %d\n", SB.posInodoRaiz);
    fprintf(stdout,"posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    fprintf(stdout,"cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    fprintf(stdout,"cantInodosLibres = %d\n", SB.cantInodosLibres);
    fprintf(stdout,"totBloques = %d\n", SB.totBloques);
    fprintf(stdout,"totInodos = %d\n", SB.totInodos);

#if DEBUGN2
    fprintf(stdout,"\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contlibres = 0;
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        if (bread(i, inodos) == FALLO) {
            return FALLO;
        }
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            if ((inodos[j].tipo == 'l')) {
                contlibres++;
                if (contlibres < 20) {
                    fprintf(stdout,"%d ", contlibres);
                } else if (contlibres == 21) {
                    fprintf(stdout,"... ");
                } else if ((contlibres > 24990) && (contlibres < SB.totInodos)) {
                    fprintf(stdout,"%d ", contlibres);
                } else if (contlibres == SB.totInodos) {
                    fprintf(stdout,"-1 \n");
                }
                contlibres--;
            }
            contlibres++;
        }
    }
#endif

#if DEBUGN3
    fprintf(stdout,"\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");
    int bloqueReservado = reservar_bloque(); // Actualiza el SB
    bread(posSB, & SB); // Actualiza los valores del SB
    fprintf(stdout,"Se ha bloqueReservado el bloque físico nº %i que era el 1º libre indicado por el MB.\n", bloqueReservado);
    fprintf(stdout,"SB.cantBloquesLibres: %i\n", SB.cantBloquesLibres);
    liberar_bloque(bloqueReservado);
    bread(posSB, & SB); // Actualiza los valores del SB
    fprintf(stdout,"Liberamos ese bloque, y después SB.cantBloquesLibres: %i\n\n", SB.cantBloquesLibres);
    fprintf(stdout,"MAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    int bit = leer_bit(posSB);
    fprintf(stdout,"leer_bit(%i) = %i\n", posSB, bit);
    bit = leer_bit(SB.posPrimerBloqueMB);
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posPrimerBloqueMB, bit);
    bit = leer_bit(SB.posUltimoBloqueMB);
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posUltimoBloqueMB, bit);
    bit = leer_bit(SB.posPrimerBloqueAI);
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posPrimerBloqueAI, bit);
    bit = leer_bit(SB.posUltimoBloqueAI);
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posUltimoBloqueAI, bit);
    bit = leer_bit(SB.posPrimerBloqueDatos);
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posPrimerBloqueDatos, bit);
    bit = leer_bit(SB.posUltimoBloqueDatos);
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posUltimoBloqueDatos, bit);
    fprintf(stdout,"\nDATOS DEL DIRECTORIO RAIZ\n\n");
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
    fprintf(stdout,"tipo: %c\n", inodo.tipo);
    fprintf(stdout,"permisos: %i\n", inodo.permisos);
    fprintf(stdout,"ID: %d \nATIME: %s \nMTIME: %s \nCTIME: %s\n", ninodo, atime, mtime, ctime);
    fprintf(stdout,"nlinks: %i\n", inodo.nlinks);
    fprintf(stdout,"tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
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
    fprintf(stdout,"\nINODO %d - TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n", nInodo);
    traducir_bloque_inodo(&inodo,8,1);
    traducir_bloque_inodo(&inodo,204,1);
    traducir_bloque_inodo(&inodo,30004,1);
    traducir_bloque_inodo(&inodo,400004,1);
    traducir_bloque_inodo(&inodo,468750,1);
    fprintf(stdout,"\nDATOS DEL INODO RESERVADO: %d\n", nInodo);
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
    fprintf(stdout,"tipo: %c\n", inodo.tipo);
    fprintf(stdout,"permisos: %i\n", inodo.permisos);
    fprintf(stdout,"ATIME: %s \nMTIME: %s \nCTIME: %s\n", atime, mtime, ctime);
    fprintf(stdout,"nlinks: %i\n", inodo.nlinks);
    fprintf(stdout,"tamaño en bytes lógicos: %i\n", inodo.tamEnBytesLog);
    fprintf(stdout,"Número de bloques ocupados: %i\n", inodo.numBloquesOcupados);
    fprintf(stdout,"SB.posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
#endif

#if DEBUGN7
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1); //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
#endif

    //Liberación
    if (bumount() == FALLO)
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        return FALLO;
    }
    return EXITO;
}

