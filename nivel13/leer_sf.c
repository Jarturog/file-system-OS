// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"
void mostrar_buscar_entrada(char *camino, char reservar);
void mostrar_buscar_entrada(char *camino, char reservar) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6);
    fprintf(stderr, "\ncamino: %s, reservar: %i\n", camino, reservar);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
    }
    fprintf(stderr, "**********************************************************************\n");
}

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, ROJO "Sintaxis: ./leer_sf <nombre_dispositivo>\n" RESET);
        return FALLO;
    }

    const char *nombre_disco = argv[1];

    if (bmount(nombre_disco) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }

    struct superbloque SB; // leo el superbloque
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo el superbloque\n" RESET);
        return FALLO;
    }

    // se imprime el superbloque
    fprintf(stdout,"DATOS DEL SUPERBLOQUE\nposPrimerBloqueMB = %i\nposUltimoBloqueMB = %i\nposPrimerBloqueAI = %i\nposUltimoBloqueAI = %i\nposPrimerBloqueDatos = %i\nposUltimoBloqueDatos = %i\nposInodoRaiz = %i\nposPrimerInodoLibre = %i\ncantBloquesLibres = %i\ncantInodosLibres = %i\ntotBloques = %i\ntotInodos = %i\n",  SB.posPrimerBloqueMB, SB.posUltimoBloqueMB, SB.posPrimerBloqueAI, SB.posUltimoBloqueAI, SB.posPrimerBloqueDatos, SB.posUltimoBloqueDatos, SB.posInodoRaiz, SB.posPrimerInodoLibre, SB.cantBloquesLibres, SB.cantInodosLibres, SB.totBloques, SB.totInodos);

#if DEBUGN2
    fprintf(stdout,"\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int inodos_libres = 0, i = 0, j = 0;
    for (i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) {
        bread(i, inodos);
        for (j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            if (inodos_libres < SB.totInodos) {
                if (inodos[j].tipo == 'l') {
                    printf("%d ", inodos[j].punterosDirectos[0]);
                    inodos_libres++;
                }
            } else {
               break;
            }
        }
        bwrite(i, inodos);
        if (inodos_libres >= SB.totInodos) {
            break;
        }
    }
#endif

#if DEBUGN3
    fprintf(stdout,"\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS:\n");
    int bloqueReservado = reservar_bloque();
    if(bloqueReservado == FALLO) {
        fprintf(stderr, ROJO "Error reservando bloque" RESET);
        return FALLO;
    }
    if(bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo el superbloque" RESET);
        return FALLO;
    }
    fprintf(stdout,"Se ha reservado el bloque físico nº %i que era el 1º libre indicado por el MB.\nSB.cantBloquesLibres: %i\n", bloqueReservado, SB.cantBloquesLibres);
    if(liberar_bloque(bloqueReservado) == FALLO) {
        fprintf(stderr, ROJO "Error liberando bloque" RESET);
        return FALLO;
    }
    if(bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo el superbloque" RESET);
        return FALLO;
    }
    fprintf(stdout,"Liberamos ese bloque, y después SB.cantBloquesLibres: %i\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n", SB.cantBloquesLibres);
    int bitLeido = leer_bit(posSB);
    if(bitLeido == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit" RESET);
        return FALLO;
    }
    fprintf(stdout,"leer_bit(%i) = %i\n", posSB, bitLeido);
    bitLeido = leer_bit(SB.posPrimerBloqueMB);
    if(bitLeido == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit" RESET);
        return FALLO;
    }
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posPrimerBloqueMB, bitLeido);
    bitLeido = leer_bit(SB.posUltimoBloqueMB);
    if(bitLeido == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit" RESET);
        return FALLO;
    }
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posUltimoBloqueMB, bitLeido);
    bitLeido = leer_bit(SB.posPrimerBloqueAI);
    if(bitLeido == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit" RESET);
        return FALLO;
    }
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posPrimerBloqueAI, bitLeido);
    bitLeido = leer_bit(SB.posUltimoBloqueAI);
    if(bitLeido == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit" RESET);
        return FALLO;
    }
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posUltimoBloqueAI, bitLeido);
    bitLeido = leer_bit(SB.posPrimerBloqueDatos);
    if(bitLeido == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit" RESET);
        return FALLO;
    }
    fprintf(stdout,"leer_bit(%i) = %i\n", SB.posPrimerBloqueDatos, bitLeido);
    bitLeido = leer_bit(SB.posUltimoBloqueDatos);
    if(bitLeido == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit" RESET);
        return FALLO;
    }
    fprintf(stdout,"leer_bit(%i) = %i\n\nDATOS DEL DIRECTORIO RAIZ\n\n", SB.posUltimoBloqueDatos, bitLeido);
    struct tm *fechaYTiempoN3;
    int tamN3 = 128;
    char aN3[tamN3], mN3[tamN3], cN3[tamN3];
    struct inodo inodoN3;
    int ninodo = 0;
    if(leer_inodo(ninodo, &inodoN3) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo inodo" RESET);
        return FALLO;
    }
    fechaYTiempoN3 = localtime(&inodoN3.mtime);
    strftime(mN3, tamN3, "%Y-%m-%d %H:%M:%S", fechaYTiempoN3);
    fechaYTiempoN3 = localtime(&inodoN3.ctime);
    strftime(cN3, tamN3, "%Y-%m-%d %H:%M:%S", fechaYTiempoN3);
    fechaYTiempoN3 = localtime(&inodoN3.atime);
    strftime(aN3, tamN3, "%Y-%m-%d %H:%M:%S", fechaYTiempoN3);
    fprintf(stdout,"tipo: %c\npermisos: %i\nID: %i \nATIME: %s \nMTIME: %s \nCTIME: %s\nnlinks: %i\ntamaño en bytes lógicos: %i\nNúmero de bloques ocupados: %i\n", inodoN3.tipo, inodoN3.permisos, ninodo, aN3, mN3, cN3, inodoN3.nlinks, inodoN3.tamEnBytesLog, inodoN3.numBloquesOcupados);
#endif

#if DEBUGN4
    int nInodo = reservar_inodo('f', 6);
    if(nInodo == FALLO){
        fprintf(stderr, ROJO "Error reservando inodo" RESET);
        return FALLO;
    }
    struct inodo inodoN4;
    if(leer_inodo(nInodo, &inodoN4) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo inodo %i" RESET, nInodo);
        return FALLO;
    }
    if(bread(posSB, &SB) == FALLO){
        fprintf(stderr, ROJO "Error leyendo superbloque" RESET);
        return FALLO;
    }
    unsigned char num_bloques_logicos = 5;
    unsigned int bloques_logicos[] = {8, 204, 30004, 400004, 468750};
    fprintf(stdout,"\nINODO %i - TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n", nInodo);
    for (unsigned char i = 0; i < num_bloques_logicos; i++) {
        if (traducir_bloque_inodo(&inodoN4, bloques_logicos[i], 1) == FALLO) {
            fprintf(stderr, ROJO "Error traduciendo bloque %i" RESET, bloques_logicos[i]);
            return FALLO;
        }
    }
    fprintf(stdout,"\nDATOS DEL INODO RESERVADO: %i\n", nInodo);
    struct tm *fechaYTiempoN4;
    int tamN4 = 128;
    char aN4[tamN4], mN4[tamN4], cN4[tamN4];
    fechaYTiempoN4 = localtime(&inodoN4.mtime);
    strftime(mN4, tamN4, "%Y-%m-%d %H:%M:%S", fechaYTiempoN4);
    fechaYTiempoN4 = localtime(&inodoN4.ctime);
    strftime(cN4, tamN4, "%Y-%m-%d %H:%M:%S", fechaYTiempoN4);
    fechaYTiempoN4 = localtime(&inodoN4.atime);
    strftime(aN4, tamN4, "%Y-%m-%d %H:%M:%S", fechaYTiempoN4);
    fprintf(stdout,"tipo: %c\npermisos: %i\nATIME: %s \nMTIME: %s \nCTIME: %s\nnlinks: %i\ntamaño en bytes lógicos: %i\nNúmero de bloques ocupados: %i\nSB.posPrimerInodoLibre = %i\n", inodoN4.tipo, inodoN4.permisos, aN4, mN4, cN4, inodoN4.nlinks, inodoN4.tamEnBytesLog, inodoN4.numBloquesOcupados, SB.posPrimerInodoLibre);
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

    if (bumount() == FALLO) {
        fprintf(stderr, ROJO "Error desmontando el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}

