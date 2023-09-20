// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "ficheros_basico.h"

/**
 * Calcula el tamaño en bloques necesario para el mapa de bits.
 * nbloques: cantidad de bloques del SF.
 * Devuelve el tamaño del mapa de bits en bloques.
*/
int tamMB(unsigned int nbloques) {
    int tamBloqueEnBits = 8 * BLOCKSIZE; // (nbloques / 8bits) / BLOCKSIZE = nbloques / (8bits * BLOCKSIZE)
    int bloqueExtra = nbloques % tamBloqueEnBits; 
    if (bloqueExtra != 0) {
        bloqueExtra = 1;
    } // si no se puede dividr sin residuo se redondeará hacia arriba el resultado final
    return (nbloques / tamBloqueEnBits) + bloqueExtra;
    // return (nbloques / (8 * BLOCKSIZE)) + !(!(nbloques % (8 * BLOCKSIZE))); // línea equivalente a los cálculos anteriores
}

/**
 * Calcula el tamaño en bloques del array de inodos.
 * ninodos: número de inodos que tendrá el array de inodos.
 * Devuelve el tamaño del array de inodos.
*/
int tamAI(unsigned int ninodos) { // ninodos = nbloques / 4
    int bloquesAI = (ninodos * INODOSIZE) / BLOCKSIZE;
    if ((ninodos * INODOSIZE) % BLOCKSIZE != 0) {
        bloquesAI++;
    } // si no se puede dividr sin residuo se redondeará hacia arriba el resultado final
    return bloquesAI;
    // return ((ninodos * INODOSIZE) / BLOCKSIZE) + !(!((ninodos * INODOSIZE) % BLOCKSIZE)); // línea equivalente a los cálculos anteriores
}

/**
 * Inicializa los datos del superbloque.
 * nbloques: número de bloques del SF.
 * ninodos: número de inodos del SF.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a bwrite() para almacenar el superbloque en memoria secundaria.
*/
int initSB(unsigned int nbloques, unsigned int ninodos) {
    struct superbloque SB;
    // Definición de los valores de los campos del superbloque
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
    // Escribe la estructura del superbloque en el bloque 0 (posSB)
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir el superbloque\n" RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a bread(), bwrite(), memset(), tamMB() y tamAI();
*/
int initMB() {
    unsigned int i;
    unsigned int tamanyoMB;
    unsigned int tamanyoAI;
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el SB\n" RESET);
        return FALLO;
    }
    tamanyoMB = tamMB(SB.totBloques);
    tamanyoAI = tamAI(SB.totInodos);
    unsigned int tamMetadatos = tamSB + tamanyoMB + tamanyoAI;
    // Calcula el número de bytes necesarios para representar los metadatos
    unsigned int numBitsMetadatos = tamMetadatos / 8;
    // Si los metadatos no caben en un bloque
    if (numBitsMetadatos / BLOCKSIZE > 1) {
        unsigned int numBloquesMB = numBitsMetadatos / BLOCKSIZE;
        for (i = 0; i < numBloquesMB; i++) {
            if (memset(bufferMB, 255, BLOCKSIZE) == NULL) { // (aquí se hace lo mismo una y otra vez)!!!
                fprintf(stderr, ROJO "Error al escribir el bloque %d del MB\n" RESET, SB.posPrimerBloqueMB + i);
                return FALLO;
            }
            if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == FALLO) {
                fprintf(stderr, ROJO "Error al escribir el bloque %d del MB\n" RESET, SB.posPrimerBloqueMB + i);
                return FALLO;
            }
        }
    }
    else {
        // Pone a 1 los bits que representan los metadatos (SB, MB y AI)
        for (i = 0; i < numBitsMetadatos; i++) { // (aquí se podría hacer un MEMSET)!!!
            bufferMB[i] = 255; // En binario, 255 es 11111111, lo que pone todos los bits a 1
        }
    }
    // Si hay bits de metadatos que no caben en un byte completo, se inicializa el byte adicional con los bits restantes
    if (tamMetadatos % 8 != 0) {
        int numBitsRestantes = tamMetadatos % 8;
        unsigned char ultimoByte = 0;
        for (i = 0; i < numBitsRestantes; i++) {
            ultimoByte |= (1 << (7 - i)); // Pone los bits restantes a 1
        }
        bufferMB[numBitsMetadatos] = ultimoByte;
        numBitsMetadatos++; // Se incrementa el número de bytes a inicializar
    }
    // Pone a 0 los bytes restantes del bloque
    for (i = numBitsMetadatos; i < BLOCKSIZE; i++) { // (aquí se podría hacer un MEMSET)!!!
        bufferMB[i] = 0; // En binario, 0 es 00000000, lo que pone todos los bits a 0
    }
    if (bwrite(SB.posPrimerBloqueMB, bufferMB) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir el bloque %d del MB\n" RESET, SB.posPrimerBloqueMB);
        return FALLO;
    }
    SB.cantBloquesLibres -= tamMetadatos; // Se actualiza la cantidad de bloques libres
    // Pone a 0 los bits del campo de datos
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir el SB\n" RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Inicializa la lista de inodos libres.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a bread() y bwrite().
*/
int initAI() {
    struct inodo inodos[BLOCKSIZE / INODOSIZE]; // inodos por bloque, 8
    // Leemos el superbloque para obtener la localización del array de inodos
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque\n" RESET);
        return FALLO;
    }
    // inodos[0].tipo = 'l';
    // inodos[0].punterosDirectos[0] = 1;
    char fin = 0;
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    // Inicializamos todos los inodos enlazando cada uno con el siguiente
    for (int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && !fin; i++) {
        if (bread(i, inodos) == FALLO) {
            fprintf(stderr, ROJO "Error al leer el bloque de inodos %d\n" RESET, i);
            return FALLO;
        }
        for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++) { // para cada inodo del AI
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos) { // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            }
            else {
                inodos[j].punterosDirectos[0] = UINT_MAX; // hemos llegado al último inodo
                fin = !fin;
                break; // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
            }
        }
        if (bwrite(i, inodos) == FALLO) {
            fprintf(stderr, ROJO "Error al escribir el bloque de inodos\n" RESET);
            return FALLO;
        }
    }
    return EXITO;
}

/**
 * Escribe el valor indicado por el parámetro bit: 0 (libre) ó 1 (ocupado) en un determinado bit del MB que representa el bloque nbloque.
 * nbloque: número de bloque físico que especificaremos is está libre o no.
 * bit: nuevo estado del bloque nbloque. Se escribe en el MB.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a bread() y bwrite().
*/
int escribir_bit(unsigned int nbloque, unsigned int bit) {
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque\n" RESET);
        return FALLO;
    }
    // Cálculo de posbyte y posbit
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    // Cálculo de nbloqueMB y nbloqueabs
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    // Leer el bloque del MB y almacenarlo en bufferMB
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el bufferMB\n" RESET);
        return FALLO;
    }
    // Modificar el bit correspondiente en bufferMB
    posbyte = posbyte % BLOCKSIZE; // Ajuste de posbyte al rango del buffer
    unsigned char mascara = 128;   // 10000000
    mascara >>= posbit;            // Desplazamiento de bits
    if (bit == 1) {
        bufferMB[posbyte] |= mascara; // Operador OR
    }
    else {
        bufferMB[posbyte] &= ~mascara; // Operadores AND y NOT
    }
    // Escribir el bloque del MB modificado en el dispositivo
    if (bwrite(nbloqueabs, bufferMB) == FALLO) {
        fprintf(stderr, ROJO "Error al esbribir el bloque absoluto en el dispositivo\n" RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Lee un determinado bit del MB.
 * nbloque: número de bloque físico que especificaremos is está libre o no.
 * Devuelve el valor del bit leído o FALLO si ha habido un error.
 * 
 * Internamente llama a bread().
*/
char leer_bit(unsigned int nbloque) {
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo bit\n" RESET);
        return FALLO;
    }
    // Obtener el byte del MB que contiene el bit deseado
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    // Cálculo de nbloqueMB y nbloqueabs
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el bloque del MB\n" RESET);
        return FALLO;
    }
    // Aplicar la máscara y desplazamientos de bits a la derecha para leer el bit
    unsigned char mascara = 128; // 10000000 en binario
    mascara >>= posbit;          // Desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte % BLOCKSIZE];
    mascara >>= (7 - posbit); // Desplazamiento a la derecha
    return mascara;
}

/**
 * Encuentra el primer bloque libre, consultando el MB, lo ocupa y devuelve su posición.
 * Devuelve la posición del primer bloque libre o FALLO si ha habido un error.
 * 
 * Internamente llama a bread(), bwrite(), memset(), memcmp() y escribir_bit().
*/
int reservar_bloque() {
    // Leemos el superbloque para obtener la cantidad de bloques libres
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque\n" RESET);
        return FALLO;
    }
    // Comprobamos si hay bloques libres
    if (SB.cantBloquesLibres == 0) {
        fprintf(stderr, ROJO "No hay bloques libres\n" RESET);
        return FALLO;
    }
    // Buscamos el primer bloque libre en el mapa de bits
    int nbloqueabs = SB.posPrimerBloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    if (memset(bufferAux, 255, BLOCKSIZE) == NULL) {
        fprintf(stderr, ROJO "Error reservando bloque\n" RESET);
        return FALLO;
    }
    while (nbloqueabs < SB.cantBloquesLibres) {
        if (bread(nbloqueabs, bufferMB) == FALLO) {
            fprintf(stderr, ROJO "Error al leer el mapa de bits\n" RESET);
            return FALLO;
        }
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {
            // Encontramos un bloque con al menos un bit a 0
            break;
        }
        nbloqueabs++;
    }
    int posbyte = 0, posbit = 0;
    while (posbyte < BLOCKSIZE && 255 == bufferMB[posbyte]) {
        posbyte++;
    }
    unsigned char mascara = 128; // 10000000 en binario
    while (mascara & bufferMB[posbyte]) {
        posbit++;
        bufferMB[posbyte] <<= 1; // desplazo a la izquierda los bits
    }
    // Calculamos el número de bloque físico
    int nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    // Reservamos el bloque en el mapa de bits
    if (escribir_bit(nbloque, 1) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir el bit en el MB\n" RESET);
        return FALLO;
    }
    // Actualizamos la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres--;
    // Lo guardamos en el superbloque
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir en el superbloque\n" RESET);
        return FALLO;
    }
    // Limpiamos el bloque en la zona de datos
    unsigned char buffer[BLOCKSIZE];
    if (memset(buffer, 0, BLOCKSIZE) == NULL) {
        fprintf(stderr, ROJO "Error reservando bloque\n" RESET);
        return FALLO;
    }
    if (bwrite(nbloque + SB.posPrimerBloqueDatos - 1, buffer) == FALLO) {
        fprintf(stderr, ROJO "Error al limpiar el bloque en la zona de datos\n" RESET);
        return FALLO;
    }
    // Devolvemos el número de bloque reservado
    return nbloque;
}

/**
 * Libera el bloque indicado por parámetro.
 * nbloque: número del bloque a liberar.
 * Devuelve el valor pasado por parámetro, el número del bloque, o FALLO si ha habido un error.
 * 
 * Internamente llama a bread(), bwrite() y escribir_bit().
*/
int liberar_bloque(unsigned int nbloque) {
    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque\n" RESET);
        return FALLO;
    }
    // Ponemos a 0 el bit correspondiente al nbloque
    if (escribir_bit(nbloque, 0) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir el bit en el MB\n" RESET);
        return FALLO;
    }
    // Actualizamos la cantidad de bloques libres y lo guardamos en el superbloque
    SB.cantBloquesLibres++;
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir en el superbloque\n" RESET);
        return FALLO;
    }
    return nbloque;
}

/**
 * Escribe el contenido de una variable de tipo struct inodo, pasada por referencia, en un determinado inodo del array de inodos.
 * ninodo: número del inodo.
 * inodo: variable que va a ser escrita (su contenido) en memoria secundaria.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a bread(), bwrite() y memcpy().
*/
int escribir_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    // Leemos el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque\n" RESET);
        return FALLO;
    }
    int nblogico = ninodo / (BLOCKSIZE / INODOSIZE); // Numero de bloque lógico en el array de inodos
    int offset = ninodo % (BLOCKSIZE / INODOSIZE);   // Offset dentro del bloque lógico
    int posInodo = nblogico + SB.posPrimerBloqueAI;
    struct inodo buffer[BLOCKSIZE / INODOSIZE];
    // Leemos el bloque del array de inodos que contiene el inodo a escribir
    if (bread(posInodo, buffer) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el bloque del array de inodos\n" RESET);
        return FALLO;
    }
    // Escribimos el inodo en su posición correspondiente del buffer
    memcpy(&buffer[offset], inodo, INODOSIZE);
    // Escribimos el bloque de inodos modificado en el dispositivo virtual
    if (bwrite(posInodo, buffer) == FALLO) {
        fprintf(stderr, ROJO "Error al escribir el bloque del array de inodos en el dispositivo virtual\n" RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 * Lee un determinado inodo del array de inodos para volcarlo en una variable de tipo struct inodo pasada por referencia.
 * ninodo: número del inodo.
 * inodo: variable donde se almacenará el inodo leído de memoria secundaria.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a bread().
*/
int leer_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    int posArrayInodos, posInodo;
    // Leemos el superbloque para obtener la posición del array de inodos
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque\n" RESET);
        return FALLO;
    }
    // Calculamos el número de bloque del array de inodos que tiene el inodo solicitado
    posArrayInodos = (INODOSIZE * ninodo / BLOCKSIZE) + SB.posPrimerBloqueAI; // (ninodo / (BLOCKSIZE / INODOSIZE)) == INODOSIZE * ninodo / BLOCKSIZE
    // Leemos el bloque que contiene el ninodo
    struct inodo bufferInodos[BLOCKSIZE / INODOSIZE];
    if (bread(posArrayInodos, bufferInodos) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el bloque que contiene el inodo solicitado\n" RESET);
        return FALLO;
    }
    posInodo = ninodo % (BLOCKSIZE / INODOSIZE); // Obtenemos la posición del inodo dentro del bloque
    *inodo = bufferInodos[posInodo];             // memcpy(inodo, &bufferInodos[posInodo], sizeof(struct inodo));  // Copiamos el inodo del buffer al inodo pasado por referencia
    return EXITO;
}

/**
 * Encuentra el primer inodo libre, lo reserva, devuelve su número y actualiza la lista enlazada de inodos libres.
 * tipo: tipo en el que estará el inodo reservado.
 * permisos: permisos que tendrá el inodo.
 * Devuelve la posición del inodo reservado o FALLO si ha habido un error.
 * 
 * Internamente llama a bread(), bwrite(), memset(), leer_inodo() y escribir_inodo().
*/
int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    // Leemos el superbloque para obtener la información necesaria
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque\n" RESET);
        return FALLO;
    }
    // Comprobamos si hay inodos libres
    if (SB.cantInodosLibres < 1) {
        fprintf(stderr, ROJO "Error: no hay inodos libres\n" RESET);
        return FALLO;
    }
    // Actualizamos la lista enlazada de inodos libres
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    struct inodo inodoReservado;
    if (leer_inodo(posInodoReservado, &inodoReservado) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo el superbloque\n" RESET);
        return FALLO;
    }
    SB.posPrimerInodoLibre = inodoReservado.punterosDirectos[0];
    SB.cantInodosLibres--;
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error actualizando el superbloque\n" RESET);
        return FALLO;
    }
    // Inicializamos los campos del inodo reservado
    inodoReservado.tipo = tipo;
    inodoReservado.permisos = permisos;
    inodoReservado.nlinks = 1;
    inodoReservado.tamEnBytesLog = 0;
    inodoReservado.atime = time(NULL);
    inodoReservado.ctime = time(NULL);
    inodoReservado.mtime = time(NULL);
    inodoReservado.numBloquesOcupados = 0;
    // Ponemos inodos a 0
    if (memset(inodoReservado.punterosDirectos, 0, sizeof(inodoReservado.punterosDirectos)) == NULL) {
        fprintf(stderr, ROJO "Error reservando inodo\n" RESET);
        return FALLO;
    }
    if (memset(inodoReservado.punterosIndirectos, 0, sizeof(inodoReservado.punterosIndirectos)) == NULL) {
        fprintf(stderr, ROJO "Error reservando inodo\n" RESET);
        return FALLO;
    }
    // Escribimos el inodo reservado
    if (escribir_inodo(posInodoReservado, &inodoReservado) == FALLO) {
        fprintf(stderr, ROJO "Error escribiendo inodo\n" RESET);
        return FALLO;
    }
    return posInodoReservado;
}

/**
 * Obtiene el rango de punteros en el que se sitúa el bloque lógico que buscamos (0:D, 1:I0, 2:I1, 3:I2)
 * y obtenemos además la dirección almacenada en el puntero correspondiente del inodo, ptr.
 * inodo: inodo en cuestión.
 * nblogico: número del bloque lógico.
 * ptr: donde se almacenará la dirección del puntero correspondiente del inodo.
 * Devuelve el número que representa el rango en el que está el bloque lógico o FALLO si ha habido un error.
 */
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr) {
    if (nblogico < DIRECTOS) { // <12
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0) { // <268
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1) { // <65.804
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2) { // <16.843.020
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    *ptr = 0;
    fprintf(stderr, ROJO "Bloque lógico fuera de rango\n" RESET);
    return FALLO;
}

/**
 * Obtiene el índice del bloque de punteros de un inodo.
 * nivel_punteros: entero que representa el nivel en el que se encuentran los punteros.
 * nblogico: número del bloque lógico.
 * Devuelve el índice del bloque de punteros de un inodo o FALLO si ha habido un error.
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros) {
    if (nblogico < DIRECTOS) {
        return nblogico;
    }
    else if (nblogico < INDIRECTOS0) {
        return nblogico - DIRECTOS;
    }
    else if (nblogico < INDIRECTOS1) {
        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2) {
        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    fprintf(stderr, ROJO "nblógico %i mayor que cualquier tipo de puntero\n" RESET, nblogico);
    return FALLO; // si ha llegado hasta aquí es porque ha habido un error
}

/**
 * Obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
 * inodo: inodo indicado.
 * nblogico: número del bloque lógico.
 * reservar: 0 si no se quiere reservar, 1 si se quiere reservar además de consultar.
 * Devuelve el número de bloque físico correspondiente a un bloque lógico determinado del inodo indicado o FALLO si ha habido un error.
 * 
 * Internamente llama a bread(), bwrite(), memset(), obtener_nRangoBL, reservar_bloque(), time() y obtener_indice().
 */
int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar) {
    unsigned int ptr, ptr_ant;
    int nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    ptr = 0, ptr_ant = 0;
    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    if (nRangoBL < 0) {
        fprintf(stderr, ROJO "Error obteniendo el número del rango del bloque en traducir_bloque_inodo()\n" RESET);
        return FALLO;
    }
    nivel_punteros = nRangoBL; // el nivel_punteros +alto es el que cuelga directamente del inodo
    while (nivel_punteros > 0) { // iterar para cada nivel de punteros indirectos
        if (ptr == 0) { // no cuelgan bloques de punteros
            if (reservar == 0) {
                return FALLO;           // bloque inexistente -> no imprimir error por pantalla!!!
            }
            // else reservar bloques de punteros y crear enlaces desde el inodo hasta el bloque de datos de punteros                      
            ptr = reservar_bloque();
            if (ptr == FALLO) {
                fprintf(stderr, ROJO "Error reservando bloque en traducir_bloque_inodo()\n" RESET);
                return FALLO;
            }
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL); // fecha actual
            if (nivel_punteros == nRangoBL) { // el bloque cuelga directamente del inodo
                inodo->punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUGTRADBI
                    fprintf(stderr, GRIS "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n" RESET,
                           nRangoBL - 1, ptr, ptr, nivel_punteros);
#endif
            }
            else { // el bloque cuelga de otro bloque de punteros
                buffer[indice] = ptr;
                if (bwrite(ptr_ant, buffer) == FALLO) { // salvamos en el dispositivo el buffer de punteros modificado
                    fprintf(stderr, ROJO "Error salvando en el dispositivo el buffer de punteros modificado en traducir_bloque_inodo()\n" RESET);
                    return FALLO;
                }
#if DEBUGTRADBI
                    fprintf(stderr, GRIS "[traducir_bloque_inodo()→ inodo.punteros_nivel%i[%i] = %i (reservado BF %i para punteros_nivel%i)]\n" RESET,
                           nivel_punteros, indice, ptr, ptr, nivel_punteros);
#endif
            }
            if (memset(buffer, 0, BLOCKSIZE) == NULL) { // ponemos a 0 todos los punteros del buffer
                fprintf(stderr, ROJO "Error poniendo a 0s el buffer en traducir_bloque_inodo()\n" RESET);
                return FALLO;
            }
        }
        else if (bread(ptr, buffer) == FALLO) { // leemos del dispositivo el bloque de punteros ya existente
            fprintf(stderr, ROJO "Error leyendo del dispositivo el bloque de punteros en traducir_bloque_inodo()\n" RESET);
            return FALLO;
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        if (indice == FALLO) {
            fprintf(stderr, ROJO "Error obteniendo índice en traducir_bloque_inodo()\n" RESET);
            return FALLO;
        }
        ptr_ant = ptr;        // guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } // al salir de este bucle ya estamos al nivel de datos
    if (ptr == 0) { // no existe bloque de datos
        if (reservar == 0) {
            return FALLO; // error lectura no existe bloque
        }
        ptr = reservar_bloque(); // de datos
        if (ptr == FALLO) {
            fprintf(stderr, ROJO "Error reservando bloque en traducir_bloque_inodo()\n" RESET);
            return FALLO;
        }
        inodo->numBloquesOcupados++;
        inodo->ctime = time(NULL);
        if (nRangoBL == 0) {                         // si era un puntero Directo
            inodo->punterosDirectos[nblogico] = ptr; // asignamos la direción del bl. de datos en el inodo
#if DEBUGTRADBI
                fprintf(stderr, GRIS "[traducir_bloque_inodo()→ inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n" RESET,
                       nblogico, ptr, ptr, nblogico);
#endif
        }
        else {
            buffer[indice] = ptr; // asignamos la dirección del bloque de datos en el buffer
            if (bwrite(ptr_ant, buffer) == FALLO) { // salvamos en el dispositivo el buffer de punteros modificado
                fprintf(stderr, ROJO "Error salvando en el dispositivo el buffer de punteros modificado en traducir_bloque_inodo()\n" RESET);
                return FALLO;
            }
#if DEBUGTRADBI
                fprintf(stderr, GRIS "[traducir_bloque_inodo()→ inodo.punteros_nivel1[%i] = %i (reservado BF %i para BL %i)]\n" RESET,
                       indice, ptr, ptr, nblogico);
#endif
        }
    }
    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}

/**
 * Libera un inodo.
 * ninodo: número del inodo que se quiere liberar.
 * Devuelve el número del inodo que ha liberado o FALLO si ha habido un error.
 * 
 * Internamente llama a bread(), bwrite(), leer_inodo(), escribir_inodo(), time() y liberar_bloques_inodo().
 */
int liberar_inodo(unsigned int ninodo) {
    // Leer el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque en la función liberar_inodo en ficheros_basico.c\n" RESET);
        return FALLO;
    }
    // Comprobamos si hay inodos para liberar
    if (SB.cantInodosLibres == SB.totInodos) {
        fprintf(stderr, ROJO "Error: todos los inodos están libres\n" RESET);
        return FALLO;
    }
    // Lectura del inodo a liberar
    struct inodo inodoALiberar;
    if (leer_inodo(ninodo, &inodoALiberar) == FALLO) {
        fprintf(stderr, ROJO "Error leyendo el inodo a liberar en la función liberar_inodo en ficheros_basico.c\n" RESET);
        return FALLO;
    }
    // Llamar a la función auxiliar liberar_bloques_inodo() para liberar todos los bloques del inodo.
    // El argumento primerBL que le pasamos, valdrá 0 cuando la llamamos desde esta función, ya que liberamos TODOS los bloques ocupados.
    int bloquesLiberados = liberar_bloques_inodo(0, &inodoALiberar);
    if(bloquesLiberados == FALLO) {
        fprintf(stderr, ROJO "Error liberando los bloques del inodo a liberar en la función liberar_inodo en ficheros_basico.c\n" RESET);
        return FALLO;
    }
    // A la cantidad de bloques ocupados del inodo se le resta la cantidad de bloques liberados que nos haya devuelto la función liberar_bloques_inodo() (y debería quedar a 0)
    inodoALiberar.numBloquesOcupados -= bloquesLiberados;
    if (inodoALiberar.numBloquesOcupados != 0) {
        fprintf(stderr, ROJO "Error al liberar una cantidad diferente de bloques (%i) del inodo a liberar en la función liberar_inodo en ficheros_basico.c\n" RESET, inodoALiberar.numBloquesOcupados);
        return FALLO;
    }
    // Leer el superbloque actualizado por liberar_bloques_inodo
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error al leer el superbloque actualizado en la función liberar_inodo en ficheros_basico.c\n" RESET);
        return FALLO;
    }
    // Marcar el inodo como tipo libre y tamEnBytesLog=0
    inodoALiberar.tipo = 'l';
    inodoALiberar.tamEnBytesLog = 0;
    // Actualizar la lista enlazada de inodos 
    unsigned int posInodoAnterior = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    inodoALiberar.punterosDirectos[0] = posInodoAnterior;
    SB.cantInodosLibres++;
    // Escribir el superbloque en el dispositivo virtual.
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, ROJO "Error actualizando el superbloque\n" RESET);
        return FALLO;
    }
    // Actualizar el ctime.
    inodoALiberar.ctime = time(NULL);
    // Escribir el inodo actualizado en el dispositivo virtual.
    if (escribir_inodo(ninodo, &inodoALiberar) == FALLO) {
        fprintf(stderr, ROJO "Error escribiendo inodo\n" RESET);
        return FALLO;
    }
    return ninodo;
}

/**
 * Variables globales para el cálculo de los bloques liberados en caso de DEBUG
*/
#if DEBUGN6
int BLliberado = 0; // utilizado para imprimir el nº de bloque lógico que se ha liberado
int calculosParaBLliberado = 0;
#endif
/**
 * ¡¡¡ FUNCIÓN 100% DISEÑADA PARA EL SOLO USO DE ELLA EN liberar_bloques_inodo() !!!
 * 
 * Recorre un nivel de punteros.
 * calculosParaActualBL: valor para calcular el BL actual que dependiendo del nivel que se recorra tiene los siguientes valores
 *  {ultimoBL, ultimoBL - DIRECTOS, ultimoBL - INDIRECTOS0 - indice * NPUNTEROS, ultimoBL - INDIRECTOS1 - indice1 * NPUNTEROS2 - indice2 * NPUNTEROS}.
 * puntero: puntero que recorre el nivel. Comprende dos posibles valores {bl_punteros[], inodo->punterosDirectos}.
 * modBL: puntero para saber si se ha modificado un bloque de punteros de algún nivel.
 * liberados: puntero a un entero que acumula la cantidad de bloques liberados.
 * i: índice en el que se empieza a recorrer el nivel.
 * eof: puntero a un entero que representa el momento en el que se ha llegado al EOF y no hay que seguir recorriendo.
 * fin: condición de finalización para el recorrido. Dos posibles valores {DIRECTOS, NPUNTEROS};
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a liberar_bloque().
 */
int recorrer_nivel(int calculosParaActualBL, unsigned int *puntero, int *modBL, int *liberados, int i, int *eof, unsigned int fin) {
    for (; !*eof && i < fin; i++) {
        *eof = !(calculosParaActualBL - i);
        if (puntero[i]) {
            if (liberar_bloque(puntero[i]) == FALLO) {
                fprintf(stderr, ROJO "Error haciendo un liberar_bloque en liberar_bloques_inodo\n" RESET);
                return FALLO;
            }
#if DEBUGN6
            BLliberado = calculosParaBLliberado + i;
            fprintf(stderr, GRIS "[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n" RESET, puntero[i], BLliberado);
#endif
            (*liberados)++;
            puntero[i] = 0;
            if (modBL != NULL) *modBL = 1;
        }
    }
    return EXITO;
}

/**
 * ¡¡¡ FUNCIÓN 100% DISEÑADA PARA EL SOLO USO DE ELLA EN liberar_bloques_inodo() !!!
 * 
 * Realiza los preparativos anteriores al recorrido. Consigue el índice del nivel en el que se va a hacer el recorrido o de algún nivel superior a él.
 * nivel_punteros: el nivel que va a recorrer el índice.
 * primerBL: el primer bloque lógico desde el que se va a liberar los bloques del inodo de liberar_bloques_inodo().
 * cond: resta de dos partes de una igualdad o de una inecuación.
 * tipoCond: 0 si el tipo de condición es de igualdad o 1 si es de inecuación, donde cond >= 0.
 * bl_punteros: buffer desde el que se va a leer el bloque nbloque.
 * modBL: puntero para saber si se ha modificado un bloque de punteros de algún nivel.
 * ind_primerBL: puntero que en caso de que no sea NULL obtendrá el valor del índice.
 * nbloque: número del bloque a leer.
 * nbreads: puntero a un entero que acumula la cantidad de bread() llamados.
 * Devuelve el índice calculado a partir de obtener_indice() o FALLO si ha habido un error.
 * 
 * Internamente llama a bread() y obtener_indice().
 */
int preparativos_recorrido(int nivel_punteros, unsigned int primerBL, int cond, int tipoCond, unsigned int *bl_punteros, int *modBL, int *ind_primerBL, unsigned int nbloque, int *nbreads) {
    if (bread(nbloque, bl_punteros) == FALLO) {
        fprintf(stderr, ROJO "Error haciendo un bread en liberar_bloques_inodo\n" RESET);
        return FALLO;
    }
    if (modBL != NULL) *modBL = 0;
    (*nbreads)++;
    int i = 0;
    if ((tipoCond == 0 && cond == 0) || (tipoCond == 1 && cond >= 0)) {
        i = obtener_indice(primerBL, nivel_punteros);
        if (i == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un obtener_indice en liberar_bloques_inodo\n" RESET);
            return FALLO;
        }
        if (ind_primerBL != NULL) *ind_primerBL = i;
    }
    return i;
}

/**
 * ¡¡¡ FUNCIÓN 100% DISEÑADA PARA EL SOLO USO DE ELLA EN liberar_bloques_inodo() !!!
 * 
 * Realiza los pasos finales una vez hecho un recorrido.
 * nbloque: puntero a un entero que indica el número del bloque a escribir (si bl_punteros está a 0s se pone a 0).
 * modBL: puntero para saber si se ha modificado un bloque de punteros de algún nivel.
 * ind_modBL: índice de modBL.
 * bufAux_punteros: buffer auxiliar todo a 0s.
 * bl_punteros: puntero que recorría el nivel en recorrer_nivel().
 * liberados: puntero a un entero que acumula la cantidad de bloques liberados.
 * nbwrites: puntero a un entero que acumula la cantidad de bwrite() llamados.
 * set_modBL: vale 1 cuando modBL[ind_modBL] tenga que asignarse a 1. Cualquier otro valor en caso contrario.
 * Devuelve FALLO o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a memcmp(), bwrite() y liberar_bloque().
 */
int conclusiones_recorrido(unsigned int *nbloque, int *modBL, unsigned int ind_modBL, unsigned char *bufAux_punteros, unsigned int *bl_punteros, int *liberados, int *nbwrites, int set_modBL) {
    if (memcmp(bl_punteros, bufAux_punteros, BLOCKSIZE) == 0) {
        if (liberar_bloque(*nbloque) == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un liberar_bloque en liberar_bloques_inodo\n" RESET);
            return FALLO;
        }
#if DEBUGN6
        fprintf(stderr, GRIS "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]\n" RESET, *nbloque, ind_modBL, BLliberado);
#endif
        (*liberados)++;
        *nbloque = 0;
        if (set_modBL == 1) modBL[ind_modBL] = 1;
    }
    else if (modBL[ind_modBL - 1]) { // escribimos en el dispositivo el bloque de punteros, si ha sido modificado
        if (bwrite(*nbloque, bl_punteros) == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un bwrite en liberar_bloques_inodo\n" RESET);
            return FALLO;
        }
        (*nbwrites)++;
    }
    return EXITO;
}

/**
 * Libera todos los bloques ocupados a partir del bloque lógico indicado por el argumento primerBL (inclusive).
 * primerBL: el primer bloque lógico desde el que se quiere liberar, 0 si se quieren liberar todos, diferente de 0 si se quiere truncar.
 * inodo: inodo del cual se van a liberar sus bloques desde primerBL hasta inodo.tamEnBytesLog.
 * Devuelve el número de bloques liberados o FALLO si ha habido un error.
 * 
 * Internamente llama a memset(), obtener_indice(), recorrer_nivel(), preparativos_recorrido() y conclusiones_recorrido().
*/
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    unsigned char bufAux_punteros[BLOCKSIZE];
    unsigned int bl_punteros[3][NPUNTEROS];
    int ind_primerBL[3];      // indices del primerBL para cuando se llama desde mi_truncar_f()
    int eof = 0;              // para determinar si hemos llegado al último BL
    int nbreads = 0;          // para comprobar optimización eficiencia
    int nbwrites = 0;         // para comprobar optimización eficiencia
    int modBL[3] = {0, 0, 0}; // para saber si se ha modificado un bloque de punteros de algún nivel
    if (inodo->tamEnBytesLog == 0) return 0;
    unsigned int ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE) - !(inodo->tamEnBytesLog % BLOCKSIZE);
#if DEBUGN6
    fprintf(stderr, GRIS "[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n" RESET, primerBL, ultimoBL);
#endif
    int liberados = 0;
    memset(bufAux_punteros, 0, BLOCKSIZE);
    // liberamos los bloques de datos de punteros directos
    int ptrLVL = 0;
    if (primerBL < DIRECTOS) {
        int i = obtener_indice(primerBL, ptrLVL);
        if (i == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un obtener_indice en directos\n" RESET);
            return FALLO;
        }
        if (recorrer_nivel(ultimoBL, inodo->punterosDirectos, NULL, &liberados, i, &eof, DIRECTOS) == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un recorrer_nivel en directos\n" RESET);
            return FALLO;
        }
    }
    // liberamos los bloques de datos e índice de Indirectos[0]
    ptrLVL = 1;
    if (primerBL < INDIRECTOS0 && !eof && inodo->punterosIndirectos[0]) {
        int i = preparativos_recorrido(ptrLVL, primerBL, primerBL - DIRECTOS, 1, bl_punteros[ptrLVL - 1], &modBL[ptrLVL - 1], NULL, inodo->punterosIndirectos[0], &nbreads);
        if (i == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un preparativos_recorrido en indirectos0\n" RESET);
            return FALLO;
        }
#if DEBUGN6
        calculosParaBLliberado = DIRECTOS;
#endif
        if (recorrer_nivel(ultimoBL - DIRECTOS, bl_punteros[ptrLVL - 1], &modBL[ptrLVL - 1], &liberados, i, &eof, NPUNTEROS) == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un recorrer_nivel en indirectos0\n" RESET);
            return FALLO;
        }
        if (conclusiones_recorrido(&inodo->punterosIndirectos[0], modBL, ptrLVL, bufAux_punteros, bl_punteros[ptrLVL - 1], &liberados, &nbwrites, 0) == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un conclusiones_recorrido en indirectos0\n" RESET);
            return FALLO;
        }
    }
    // liberamos los bloques de datos e índice de Indirectos[1]
    ptrLVL = 2;
    ind_primerBL[0] = 0;
    ind_primerBL[1] = 0;
    if (primerBL < INDIRECTOS1 && !eof && inodo->punterosIndirectos[1]) {
        int i = preparativos_recorrido(ptrLVL, primerBL, primerBL - INDIRECTOS0, 1, bl_punteros[ptrLVL - 1], &modBL[ptrLVL - 1], &ind_primerBL[ptrLVL - 1], inodo->punterosIndirectos[1], &nbreads);
        if (i == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un preparativos_recorrido en indirectos1\n" RESET);
            return FALLO;
        }
        for (; !eof && i < NPUNTEROS; i++) {
            if (bl_punteros[ptrLVL - 1][i]) {
                int j = preparativos_recorrido(ptrLVL - 1, primerBL, i - ind_primerBL[ptrLVL - 1], 0, bl_punteros[ptrLVL - 2], &modBL[ptrLVL - 2], &ind_primerBL[ptrLVL - 2], bl_punteros[ptrLVL - 1][i], &nbreads);
                if (j == FALLO) {
                    fprintf(stderr, ROJO "Error haciendo un preparativos_recorrido en indirectos1\n" RESET);
                    return FALLO;
                }
#if DEBUGN6
                calculosParaBLliberado = INDIRECTOS0 + i * NPUNTEROS;
#endif
                if (recorrer_nivel(ultimoBL - INDIRECTOS0 - i * NPUNTEROS, bl_punteros[ptrLVL - 2], &modBL[ptrLVL - 2], &liberados, j, &eof, NPUNTEROS) == FALLO) {
                    fprintf(stderr, ROJO "Error haciendo un recorrer_nivel en indirectos1\n" RESET);
                    return FALLO;
                }
                if (conclusiones_recorrido(&bl_punteros[ptrLVL - 1][i], modBL, ptrLVL - 1, bufAux_punteros, bl_punteros[ptrLVL - 2], &liberados, &nbwrites, 1) == FALLO) {
                    fprintf(stderr, ROJO "Error haciendo un conclusiones_recorrido en indirectos1\n" RESET);
                    return FALLO;
                }
            }
        }
        if (conclusiones_recorrido(&inodo->punterosIndirectos[1], modBL, ptrLVL, bufAux_punteros, bl_punteros[ptrLVL - 1], &liberados, &nbwrites, 0) == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un conclusiones_recorrido en indirectos1\n" RESET);
            return FALLO;
        }
    }
    // liberamos los bloques de datos e índice de Indirectos[2]
    ptrLVL = 3;
    ind_primerBL[0] = 0;
    ind_primerBL[1] = 0;
    ind_primerBL[2] = 0;
    if (primerBL < INDIRECTOS2 && !eof && inodo->punterosIndirectos[2]) {
        int i = preparativos_recorrido(ptrLVL, primerBL, primerBL - INDIRECTOS1, 1, bl_punteros[ptrLVL - 1], &modBL[ptrLVL - 1], &ind_primerBL[ptrLVL - 1], inodo->punterosIndirectos[2], &nbreads);
        if (i == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un preparativos_recorrido en indirectos2\n" RESET);
            return FALLO;
        }
        for (; !eof && i < NPUNTEROS; i++) {
            if (bl_punteros[ptrLVL - 1][i]) {
                int j = preparativos_recorrido(ptrLVL - 1, primerBL, i - ind_primerBL[ptrLVL - 1], 0, bl_punteros[ptrLVL - 2], NULL, &ind_primerBL[ptrLVL - 2], bl_punteros[ptrLVL - 1][i], &nbreads);
                if (j == FALLO) {
                    fprintf(stderr, ROJO "Error haciendo un preparativos_recorrido en indirectos2\n" RESET);
                    return FALLO;
                }
                for (; !eof && j < NPUNTEROS; j++) {
                    if (bl_punteros[ptrLVL - 2][j]) {
                        int k = preparativos_recorrido(ptrLVL - 2, primerBL, (i - ind_primerBL[ptrLVL - 1]) * (BLOCKSIZE / 4) + j - ind_primerBL[ptrLVL - 2], 0, bl_punteros[ptrLVL - 3], NULL, &ind_primerBL[ptrLVL - 3], bl_punteros[ptrLVL - 2][j], &nbreads); // la multiplicación por BLOCKSIZE/4 (número arbitrariamente grande) es para asegurarse de que ambos sean 0, un AND
                        if (k == FALLO) {
                            fprintf(stderr, ROJO "Error haciendo un preparativos_recorrido en indirectos2\n" RESET);
                            return FALLO;
                        }
#if DEBUGN6
                        calculosParaBLliberado = INDIRECTOS1 + i * NPUNTEROS2 + j * NPUNTEROS;
#endif
                        if (recorrer_nivel(ultimoBL - INDIRECTOS1 - i * NPUNTEROS2 - j * NPUNTEROS, bl_punteros[ptrLVL - 3], &modBL[ptrLVL - 3], &liberados, k, &eof, NPUNTEROS) == FALLO) {
                            fprintf(stderr, ROJO "Error haciendo un recorrer_nivel en indirectos2\n" RESET);
                            return FALLO;
                        }
                        if (conclusiones_recorrido(&bl_punteros[ptrLVL - 2][j], modBL, ptrLVL - 2, bufAux_punteros, bl_punteros[ptrLVL - 3], &liberados, &nbwrites, 1) == FALLO) {
                            fprintf(stderr, ROJO "Error haciendo un conclusiones_recorrido en indirectos2\n" RESET);
                            return FALLO;
                        }
                    }
                }
                if (conclusiones_recorrido(&bl_punteros[ptrLVL - 1][i], modBL, ptrLVL - 1, bufAux_punteros, bl_punteros[ptrLVL - 2], &liberados, &nbwrites, 1) == FALLO) {
                    fprintf(stderr, ROJO "Error haciendo un conclusiones_recorrido en indirectos2\n" RESET);
                    return FALLO;
                }
            }
        }
        if (conclusiones_recorrido(&inodo->punterosIndirectos[2], modBL, ptrLVL, bufAux_punteros, bl_punteros[ptrLVL - 1], &liberados, &nbwrites, 0) == FALLO) {
            fprintf(stderr, ROJO "Error haciendo un conclusiones_recorrido en indirectos2\n" RESET);
            return FALLO;
        }
    }
#if DEBUGN6
    fprintf(stderr, GRIS "[liberar_bloques_inodo()→ total bloques liberados: %d, total breads: %d, total_bwrites:%d]\n" RESET, liberados, nbreads, nbwrites);
#endif
    return liberados;
}
