
#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){
    int tamBloqueEnBits = 8 * BLOCKSIZE; // (nbloques / 8bits) / BLOCKSIZE = nbloques / (8bits * BLOCKSIZE)
    int bloqueExtra = nbloques % tamBloqueEnBits;
    if (bloqueExtra != 0)
    {
        bloqueExtra = 1;
    } // si no se puede dividr sin residuo se redondeará hacia arriba el resultado final
    return (nbloques / tamBloqueEnBits) + bloqueExtra;
}
int tamAI(unsigned int ninodos){  // ninodos = nbloques / 4
    int bloquesAI = (ninodos * INODOSIZE) / BLOCKSIZE; 
    if ((ninodos * INODOSIZE) % BLOCKSIZE  != 0) {
        bloquesAI++;
    }
    return bloquesAI;
}
int initSB(unsigned int nbloques, unsigned int ninodos){
    
    struct superbloque SB;
    // Definición de los valores de los campos del superbloque
    SB.posPrimerBloqueMB = posSB + tamSB;
    
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB +  tamMB(nbloques) - 1;
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
        fprintf(stderr, "Error al escribir el superbloque\n");
        return FALLO;
    }

    return EXITO;
}

int initMB() {
    unsigned int i;
    unsigned int tamanyoMB;
    unsigned int tamanyoAI;
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;
    
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, "Error al leer el SB\n");
        return FALLO;
    }
    
    tamanyoMB = tamMB(SB.totBloques);
    tamanyoAI = tamAI(SB.totInodos);
    unsigned int tamMetadatos = tamSB + tamanyoMB + tamanyoAI;
    
    // Calcula el número de bytes necesarios para representar los metadatos
    unsigned int numBitsMetadatos = tamMetadatos / 8;
    // Si los metadatos no caben en un bloque
    if (numBitsMetadatos / BLOCKSIZE > 1){
        unsigned int numBloquesMB = numBitsMetadatos / BLOCKSIZE;
        for (i = 0; i < numBloquesMB; i++) {
    
           memset(bufferMB, 255, BLOCKSIZE);
           if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == FALLO) {
           fprintf(stderr, "Error al escribir el bloque %d del MB\n", SB.posPrimerBloqueMB + i);
           return FALLO;
           }
        }
    }else{
    // Pone a 1 los bits que representan los metadatos (SB, MB y AI)
        for (i = 0; i < numBitsMetadatos; i++) {
            bufferMB[i] = 255; // En binario, 255 es 11111111, lo que pone todos los bits a 1
        }
    }
 
    // Si hay bits de metadatos que no caben en un byte completo, se inicializa el byte adicional con los bits restantes
    if (tamMetadatos % 8 != 0) {
        int numBitsRestantes = tamMetadatos % 8;
        unsigned char ultimoByte = 0;
        for (i = 0; i < numBitsRestantes; i++) {
            ultimoByte |= (1 << (7-i)); // Pone los bits restantes a 1
        }
        bufferMB[numBitsMetadatos] = ultimoByte;
        numBitsMetadatos++; // Se incrementa el número de bytes a inicializar
    }
    
    // Pone a 0 los bytes restantes del bloque
    for (i = numBitsMetadatos; i < BLOCKSIZE; i++) {
        bufferMB[i] = 0; // En binario, 0 es 00000000, lo que pone todos los bits a 0
    }

    if (bwrite(SB.posPrimerBloqueMB, bufferMB) == FALLO) {
        fprintf(stderr, "Error al escribir el bloque %d del MB\n", SB.posPrimerBloqueMB);
        return FALLO;
    }
    SB.cantBloquesLibres -= tamMetadatos; // Se actualiza la cantidad de bloques libres
    // Pone a 0 los bits del campo de datos 
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, "Error al escribir el SB\n");
        return FALLO;
    }
    return EXITO;
}


int initAI() {

    struct inodo inodos[BLOCKSIZE / INODOSIZE];  // inodos por bloque, 8
    // Leemos el superbloque para obtener la localización del array de inodos
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, "Error al leer el superbloque\n");
        return FALLO; 
    }
    //inodos[0].tipo = 'l';
    //inodos[0].punterosDirectos[0] = 1;
    char fin = 0;
    unsigned int contInodos = SB.posPrimerInodoLibre +1;
    // Inicializamos todos los inodos enlazando cada uno con el siguiente
    for(int i=SB.posPrimerBloqueAI; (i<=SB.posUltimoBloqueAI) && !fin; i++){
           if (bread(i, inodos) == FALLO) {
            fprintf(stderr, "Error al leer el bloque de inodos %d\n", i);
            return FALLO;
        }
         for (int j = 0; j < (BLOCKSIZE / INODOSIZE); j++) { // para cada inodo del AI

            inodos[j].tipo = 'l';  // libre
            
            if (contInodos < SB.totInodos) {  // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos;  // enlazamos con el siguiente 
                contInodos++;
        
            } else {
                inodos[j].punterosDirectos[0] = UINT_MAX;  // hemos llegado al último inodo
                fin = !fin;
                break;  // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
            }
        }
        if (bwrite(i, inodos) == FALLO) {
            fprintf(stderr, "Error al escribir el bloque de inodos\n");
            return FALLO;
        }
    }
    return EXITO;  
}

int escribir_bit(unsigned int nbloque, unsigned int bit) {
   struct superbloque SB;
   if (bread(posSB, &SB) == FALLO) {
       fprintf(stderr, "Error al leer el superbloque\n");
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
       fprintf(stderr, "Error al leer el bufferMB\n");
       return FALLO;
   }
   // Modificar el bit correspondiente en bufferMB
   posbyte = posbyte % BLOCKSIZE; // Ajuste de posbyte al rango del buffer
   unsigned char mascara = 128; // 10000000
   mascara >>= posbit; // Desplazamiento de bits
   if (bit == 1) {
       bufferMB[posbyte] |= mascara; // Operador OR
   } else {
       bufferMB[posbyte] &= ~mascara; // Operadores AND y NOT
   }
  
   // Escribir el bloque del MB modificado en el dispositivo
   if (bwrite(nbloqueabs, bufferMB) == FALLO){
       fprintf(stderr, "Error al esbribir el bloque absoluto en el dispositivo\n");
       return FALLO;
   }
  
   return EXITO;
}
char leer_bit(unsigned int nbloque) {
    
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
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
       fprintf(stderr, "Error al leer el bloque del MB");
       return FALLO;
   }
   // Aplicar la máscara y desplazamientos de bits a la derecha para leer el bit 
   unsigned char mascara = 128; // 10000000 en binario
   mascara >>= posbit; // Desplazamiento de bits a la derecha, los que indique posbit
   mascara &= bufferMB[posbyte % BLOCKSIZE]; 
   mascara >>= (7 - posbit);  // Desplazamiento a la derecha 
   return mascara;
}

 int reservar_bloque(){
    // Leemos el superbloque para obtener la cantidad de bloques libres
    struct superbloque SB;
    if (bread(posSB, &SB) == -1) {
        fprintf(stderr, "Error al leer el superbloque\n");
        return FALLO;
    }

    // Comprobamos si hay bloques libres
    if (SB.cantBloquesLibres == 0) {
        fprintf(stderr, "No hay bloques libres\n");
        return FALLO;
    }

    // Buscamos el primer bloque libre en el mapa de bits
    int nbloqueabs = SB.posPrimerBloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    unsigned char bufferAux[BLOCKSIZE];
    if (memset(bufferAux, 255, BLOCKSIZE) == NULL)
    {
        return FALLO;
    }
    int posbyte = -1;
    int posbit = -1;
    //while (nbloqueabs < SB.cantBloquesLibres && posbyte == -1) {
    while(0 == 0){
        if (bread(nbloqueabs, bufferMB) == FALLO) {
            fprintf(stderr, "Error al leer el mapa de bits\n");
            return FALLO;
        }
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0) {
            // Encontramos un bloque con al menos un bit a 0
            break; 
        }
        nbloqueabs++;
    }

    posbyte = 0;
    while (255 == bufferMB[posbyte]){
        posbyte++;
    }

    unsigned char mascara = 128; // 10000000 en binario
    posbit = 0;
    while (mascara & bufferMB[posbyte]){
        ++posbit;
        bufferMB[posbyte] <<= 1; // desplazo a la izquierda los bits
    }
    // Calculamos el número de bloque físico
    int nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;

    // Reservamos el bloque en el mapa de bits
    if (escribir_bit(nbloque, 1) == FALLO) {
        fprintf(stderr, "Error al escribir el bit en el MB\n");
        return FALLO;
    }

    // Actualizamos la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres--;

    // Lo guardamos en el superbloque
    if (bwrite(posSB, &SB) == FALLO) {
        fprintf(stderr, "Error al escribir en el superbloque\n");
        return FALLO;
    }

    // Limpiamos el bloque en la zona de datos
    unsigned char buffer[BLOCKSIZE];
    if(memset(buffer, 0, BLOCKSIZE) == NULL){
        return FALLO;
    }
    if (bwrite(nbloque + SB.posPrimerBloqueDatos - 1, buffer) == FALLO) {
        fprintf(stderr, "Error al limpiar el bloque en la zona de datos\n");
        return FALLO;
    }

    // Devolvemos el número de bloque reservado
    return nbloque;
 }

 int liberar_bloque(unsigned int nbloque) {
    struct superbloque SB;

    // Leemos el superbloque 
    if (bread(posSB, &SB) == FALLO) {
        printf("Error al leer el superbloque\n");
        return FALLO;
    }

    // Ponemos a 0 el bit correspondiente al nbloque
    if (escribir_bit(nbloque, 0) == FALLO) {
        printf("Error al escribir el bit en el MB\n");
        return FALLO;
    }

    // Actualizamos la cantidad de bloques libres y lo guardamos en el superbloque
    SB.cantBloquesLibres++;
    if (bwrite(posSB, &SB) == FALLO) {
        printf("Error al escribir en el superbloque\n");
        return FALLO;
    }
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo) {
    struct superbloque SB;
    // Leemos el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr,"Error al leer el superbloque\n");
        return FALLO;
    }
    
    int nblogico = ninodo / (BLOCKSIZE/INODOSIZE); // Numero de bloque lógico en el array de inodos
    int offset = ninodo % (BLOCKSIZE/INODOSIZE); // Offset dentro del bloque lógico
    int posInodo = nblogico + SB.posPrimerBloqueAI;
    
    struct inodo buffer[BLOCKSIZE/INODOSIZE];
    // Leemos el bloque del array de inodos que contiene el inodo a escribir
    if (bread(posInodo, buffer) == FALLO) {
        fprintf(stderr,"Error al leer el bloque del array de inodos\n");
        return FALLO;
    }

    // Escribimos el inodo en su posición correspondiente del buffer
    memcpy(&buffer[offset], inodo, INODOSIZE);  
    
    // Escribimos el bloque de inodos modificado en el dispositivo virtual
    if (bwrite(posInodo, buffer) == FALLO) {
        printf("Error al escribir el bloque del array de inodos en el dispositivo virtual\n");
        return FALLO;
    }
        
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo) {

    struct superbloque SB;
    int posArrayInodos, posInodo;

    // Leemos el superbloque para obtener la posición del array de inodos
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr,"Error al leer el superbloque\n");
        return FALLO;
    }
    // Calculamos el número de bloque del array de inodos que tiene el inodo solicitado
    posArrayInodos = (INODOSIZE * ninodo / BLOCKSIZE) + SB.posPrimerBloqueAI; // (ninodo / (BLOCKSIZE / INODOSIZE)) == INODOSIZE * ninodo / BLOCKSIZE
    // Leemos el bloque que contiene el ninodo
    struct inodo bufferInodos[BLOCKSIZE / INODOSIZE];
    if (bread(posArrayInodos, bufferInodos) == FALLO) {
        fprintf(stderr,"Error al leer el bloque que contiene el inodo solicitado\n");
        return FALLO;
    }
    posInodo = ninodo % (BLOCKSIZE / INODOSIZE);  // Obtenemos la posición del inodo dentro del bloque
    *inodo = bufferInodos[posInodo]; //memcpy(inodo, &bufferInodos[posInodo], sizeof(struct inodo));  // Copiamos el inodo del buffer al inodo pasado por referencia
    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos) {
    // Leemos el superbloque para obtener la información necesaria
    struct superbloque SB;
     if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr,"Error al leer el superbloque\n");
        return FALLO;
    }
    
    // Comprobamos si hay inodos libres
    if (SB.cantInodosLibres < 1) {
        fprintf(stderr, "Error: no hay inodos libres\n");
        return FALLO;
    }
    
    // Actualizamos la lista enlazada de inodos libres
    unsigned int posInodoReservado = SB.posPrimerInodoLibre;
    struct inodo inodoReservado;
    if(leer_inodo(posInodoReservado, &inodoReservado) == FALLO){
        fprintf(stderr,"Error leyendo el superbloque\n");
        return FALLO;  
    }
    SB.posPrimerInodoLibre = inodoReservado.punterosDirectos[0];
    SB.cantInodosLibres--;
    if(bwrite(posSB, &SB) == FALLO){
        fprintf(stderr,"Error actualizando el superbloque\n");
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
    //Ponemos inodos a 0
    if(memset(inodoReservado.punterosDirectos, 0, sizeof(inodoReservado.punterosDirectos)) == NULL){
        fprintf(stderr,"Error reservando inodo\n");
        return FALLO;
    }
    if(memset(inodoReservado.punterosIndirectos, 0, sizeof(inodoReservado.punterosIndirectos)) == NULL){
        fprintf(stderr,"Error reservando inodo\n");
        return FALLO;
    }
    // Escribimos el inodo reservado
    if(escribir_inodo(posInodoReservado, inodoReservado) == FALLO){
        fprintf(stderr,"Error escribiendo inodo\n");
        return FALLO;
    }
    
    return posInodoReservado;
}
/**
 * Obtiene el rango de punteros en el que se sitúa el bloque lógico que buscamos (0:D, 1:I0, 2:I1, 3:I2)
 * y obtenemos además la dirección almacenada en el puntero correspondiente del inodo, ptr.
*/
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr){
    if (nblogico < DIRECTOS) { // <12
        *ptr = inodo -> punterosDirectos[nblogico];
        return 0;
    } else if (nblogico < INDIRECTOS0) { // <268    
        *ptr = inodo -> punterosIndirectos[0];
        return 1;
    } else if (nblogico < INDIRECTOS1) { // <65.804     
        *ptr = inodo -> punterosIndirectos[1];
        return 2;
    } else if (nblogico < INDIRECTOS2) { // <16.843.020              
        *ptr = inodo -> punterosIndirectos[2];
        return 3;
    } else {
        *ptr = 0;
        fprintf(stderr, ROJO "Bloque lógico fuera de rango" RESET);
        return -1;
    }  
}

/**
 * Obtiene el índice del bloques de punteros de un inodo.
*/
int obtener_indice(unsigned int nblogico, int nivel_punteros){
    if (nblogico < DIRECTOS) {
        return nblogico; 
    } else if (nblogico < INDIRECTOS0) {
        return nblogico - DIRECTOS; 
    } else if (nblogico < INDIRECTOS1) {      
        if (nivel_punteros == 2) {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    } else if (nblogico < INDIRECTOS2) {          
        if (nivel_punteros == 3) {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        } else if (nivel_punteros == 2) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        } else if (nivel_punteros == 1) {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    fprintf(stderr, ROJO "nblógico %i mayor que cualquier tipo de puntero" RESET, nblogico);
    return FALLO; // si ha llegado hasta aquí es porque ha habido un error
}

/**
 * Obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
*/
int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar){
    unsigned int ptr, ptr_ant;
    int nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    ptr = 0, ptr_ant = 0;
    nRangoBL = obtener_nRangoBL(inodo, nblogico, & ptr); //0:D, 1:I0, 2:I1, 3:I2
    if(nRangoBL < 0){
        fprintf(stderr, ROJO "Error obteniendo el número del rango del bloque en traducir_bloque_inodo()" RESET);
        return FALLO;
    }
    nivel_punteros = nRangoBL; //el nivel_punteros +alto es el que cuelga directamente del inodo
    while (nivel_punteros > 0) { //iterar para cada nivel de punteros indirectos
        if (ptr == 0) { //no cuelgan bloques de punteros
            if (reservar == 0) {
                return -1; // bloque inexistente -> no imprimir error por pantalla!!!
            } else { //reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
                ptr = reservar_bloque(); //de punteros                  
                if(ptr == FALLO){
                    fprintf(stderr, ROJO "Error reservando bloque en traducir_bloque_inodo()" RESET);
                    return FALLO;
                }
                inodo -> numBloquesOcupados++;
                inodo -> ctime = time(NULL); //fecha actual
                if (nivel_punteros == nRangoBL) { //el bloque cuelga directamente del inodo
                    inodo -> punterosIndirectos[nRangoBL - 1] = ptr;
                } else { //el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
                    if(bwrite(ptr_ant, buffer) == FALLO){ //salvamos en el dispositivo el buffer de punteros modificado   
                        fprintf(stderr, ROJO "Error salvando en el dispositivo el buffer de punteros modificado en traducir_bloque_inodo()" RESET);
                        return FALLO;
                    }        
                }
                if(memset(buffer, 0, BLOCKSIZE) == NULL){ //ponemos a 0 todos los punteros del buffer 
                    fprintf(stderr, ROJO "Error poniendo a 0s el buffer en traducir_bloque_inodo()" RESET);
                    return FALLO;
                }
            }
        } else {
            if(bread(ptr, buffer) == FALLO){ //leemos del dispositivo el bloque de punteros ya existente
                fprintf(stderr, ROJO "Error leyendo del sipositivo el bloque de punteros en traducir_bloque_inodo()" RESET);
                return FALLO;
            }     
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        if(indice == FALLO){
            fprintf(stderr, ROJO "Error obteniendo índice en traducir_bloque_inodo()" RESET);
            return FALLO;
        }
        ptr_ant = ptr; //guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel 
        nivel_punteros--;
    } //al salir de este bucle ya estamos al nivel de datos
    if (ptr == 0) { //no existe bloque de datos
        if (reservar == 0) {
            return -1; //error lectura ∄ bloque   
        } else {
            ptr = reservar_bloque(); //de datos
            if (ptr == FALLO) {
                fprintf(stderr, ROJO "Error reservando bloque en traducir_bloque_inodo()" RESET);
                return FALLO;
            }
            inodo -> numBloquesOcupados++;
            inodo -> ctime = time(NULL);
            if (nRangoBL == 0) { //si era un puntero Directo 
                inodo -> punterosDirectos[nblogico] = ptr; //asignamos la direción del bl. de datos en el inodo
            } else {
                buffer[indice] = ptr; //asignamos la dirección del bloque de datos en el buffer
                if (bwrite(ptr_ant, buffer) == FALLO) { //salvamos en el dispositivo el buffer de punteros modificado 
                    fprintf(stderr, ROJO "Error salvando en el dispositivo el buffer de punteros modificado en traducir_bloque_inodo()" RESET);
                    return FALLO;
                }
            }
        }
    }
    //mi_write_f() se encargará de salvar los cambios del inodo en disco
    return ptr; //nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}


