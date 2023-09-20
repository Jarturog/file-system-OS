#include "ficheros_basico.h"

int tamMB(unsigned int nbloques){

    int bits = nbloques;
    int bytes = bits / 8;
    int bloquesMB = bytes / BLOCKSIZE;
    if (bytes % BLOCKSIZE != 0) {
        bloquesMB++;
    }
    return bloquesMB;
}
int tamAI(unsigned int ninodos){  //ninodos=nbloques/4
    
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
        fprintf(stderr, "Error: no se pudo escribir la estructura del superbloque en el SB\n");
        return FALLO;
    }

    return EXITO;
}

int initMB(unsigned int nbloques, unsigned int ninodos) {
    unsigned int i;
    unsigned int tamanyoMB;
    unsigned int tamanyoAI;
    tamanyoMB = tamMB(nbloques);
    tamanyoAI = tamAI(ninodos);
    unsigned int tamMetadatos = tamSB + tamanyoMB + tamanyoAI;
    
    unsigned char bufferMB[BLOCKSIZE];
    struct superbloque SB;
    
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, "Error al leer el SB\n");
        return FALLO;
    }
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


int initAI(unsigned int ninodos) {

    struct inodo inodos[BLOCKSIZE / INODOSIZE];  // inodos por bloque, 8
    // Leemos el superbloque para obtener la localización del array de inodos
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO) {
        fprintf(stderr, "Error al leer el superbloque\n");
        return FALLO; 
    }
    inodos[0].tipo = 'l';
    inodos[0].punterosDirectos[0] = 1;
    unsigned int contInodos = SB.posPrimerInodoLibre +1;
    // Inicializamos todos los inodos enlazando cada uno con el siguiente
    for(int i=SB.posPrimerBloqueAI; i<=SB.posUltimoBloqueAI; i++){
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
                break;  // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
            }
            
            
        }
        if (bwrite(SB.posPrimerBloqueAI+i, inodos) == FALLO) {
            fprintf(stderr, "Error al escribir el bloque de inodos\n");
            return FALLO;
        }

    }
    return EXITO;  
}