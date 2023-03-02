int tamMB(unsigned int nbloques)
{
    int denominador = 8 * BLOCKSIZE; // (nbloques / 8bits) / BLOCKSIZE = nbloques / (8bits * BLOCKSIZE)
    int residuo = nbloques % denominador;
    if (residuo != 0)
    {
        residuo = 1;
    } // si no se puede dividr sin residuo se redondeará hacia arriba el resultado final
    return (nbloques / denominador) + residuo;
}
int tamAI(unsigned int ninodos)
{
    int residuo = (ninodos * INODOSIZE) % BLOCKSIZE;
    if (residuo != 0)
    {
        residuo = 1;
    } // si no se puede dividr sin residuo se redondeará hacia arriba el resultado final
    return ((ninodos * INODOSIZE) / BLOCKSIZE) + residuo;
}
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    /*Inicializa los datos del superbloque.
Se trata de definir una variable de tipo struct superbloque que se vaya rellenando con la información pertinente:
Posición del primer bloque del mapa de bits
SB.posPrimerBloqueMB = posSB + tamSB //posSB = 0, tamSB = 1
SB.posPrimerBloqueMB = 0+1 = 1
Posición del último bloque del mapa de bits
SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1
tamMB(100000) = (100000/8/1024)+1 = 13
SB.posUltimoBloqueMB = 1+13-1 = 13
Posición del primer bloque del array de inodos
SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1
SB.posPrimerBloqueAI = 13+1 = 14
Posición del último bloque del array de inodos
SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1
tamAI(ninodos) = 25000*128/1024 = 3125
SB.posUltimoBloqueAI = 14+3125-1 = 3138
Posición del primer bloque de datos
SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1
SB.posPrimerBloqueDatos = 3138+1 = 3139
Posición del último bloque de datos
SB.posUltimoBloqueDatos = nbloques-1
SB.posUltimoBloqueDatos = 100000-1 = 99999

Posición del inodo del directorio raíz en el array de inodos
SB.posInodoRaiz = 0
Posición del primer inodo libre en el array de inodos
Inicialmente: SB.posPrimerInodoLibre = 0
Cuando llamemos a reservar_inodo(‘d’, 7) para crear el directorio raíz (Nivel 3), pasará a valer 1.
Posteriormente se irá actualizando para apuntar a la cabeza de la lista de inodos libres (mediante las llamadas a las funciones reservar_inodo() y liberar_inodo())
Cantidad de bloques libres en el SF
Inicialmente: SB.cantBloquesLibres = nbloques
Cuando indiquemos en el mapa de bits los bloques que ocupan los metadatos (el SB, el propio MB y el AI), restaremos esos bloques de la cantidad de bloques libres.
Al reservar un bloque ⇒ SB.cantBloquesLibres--
Al liberar un bloque ⇒ SB.cantBloquesLibres++
Cantidad de inodos libres en el array de inodos
Inicialmente: SB.cantInodosLibres = ninodos (el 1er inodo será para el directorio raíz)
Al reservar un inodo ⇒ SB.cantInodosLibres--
Al liberar un inodo ⇒ SB.cantInodosLibres++
Cantidad total de bloques
Se pasará como argumento en la línea de comandos al inicializar el sistema:

$ ./mi_mkfs <nombre_fichero> <nbloques>

y lo recibimos como parámetro
SB.totBloques = nbloques

Cantidad total de inodos
Determinada por el administrador del sistema de forma heurística (ninodos = nbloques/4) y recibida por parámetro
SB.totinodos = ninodos
Se indicará su valor en mi_mkfs.c y se pasará también como parámetro a la función de inicialización del array de inodos, initAI()

Al finalizar las inicializaciones de los campos, se escribe la estructura en el bloque posSB mediante la función bwrite().
*/
}
int initMB()
{
    /*Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
Os dejo un ejemplo de un caso para que lo generalicéis:

Si inicializamos el dispositivo con 100.000 bloques de tamaño BLOCKSIZE = 1024, ya hemos visto que los metadatos ocupan 3139 bloques (tamSB+tamMB+tamAI), por tanto hemos de poner 3139 bits a 1.
Si hacemos la operación 3139 / 8 / 1024 vemos que todos esos bits caben en un bloque, el bloque 0 del MB. Así que llevamos ese bloque a memoria usando un array de caracteres, bufferMB.
Vamos a ver cuántos de esos 1024 bytes hay que poner a 1:
3139 / 8 = 392
Para ello podemos iterar desde i = 0 a 391 haciendo la siguiente asignación:
bufferMB[i] = 255 (en binario es 11111111)
Pero aún tenemos un resto de 3 bits (3139 % 8 = 3) que habrá que poner a 1 a la izquierda en un byte adicional, en el 392. Eso implica escribir en binario 11100000, o sea 224 en decimal (2⁷+2⁶+2⁵):
bufferMB[392] = 224.
En lugar de utilizar potencias de 2, también se podrían poner esos bits a 1 usando desplazamientos
Los restantes bytes de ese bloque (desde el 393 al 1023) se tendrían que poner a 0. Y luego salvar bufferMB al dispositivo virtual, en la posición correspondiente.
También habría que tener en cuenta que los bits correspondientes a los metadatos podrían ocupar más de 1 bloque, dependiendo de la cantidad total de bloques de nuestro dispositivo virtual, nbloques, y de BLOCKSIZE. En tal caso habría que poner los bloques previos directamente a 1 (usando memset() para igualar a 255 todos los bytes del buffer de memoria y escribiéndolo en el disco), y realizar las operaciones anteriormente descritas en el bloque que sólo tuviera algunos bits a 1.


Cuando marquemos en el mapa de bits los bloques que ocupan los metadatos (el SB, el propio MB y el AI), restaremos esos bloques de la cantidad de bloques libres en el campo del superbloque, SB.cantBloquesLibres y lo salvaremos.
Esta función se testeará en el siguiente nivel cuando dispongamos de la función leer_bit().

*/
}
int initAI()
{

    struct superbloque SB;
    if (bread(posSB, &SB) == FAILURE)
    {
        return FAILURE;
    }
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int fin = 0;
    int contInodos = SB.posPrimerInodoLibre + 1;
    for (int i = SB.posPrimerBloqueAI; (i <= SB.posUltimoBloqueAI) && !fin; i++) // para cada bloque del AI
    {
        for (j = 0; j < (BLOCKSIZE / INODOSIZE); j++)
        {                         // para cada inodo del AI
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos)
            {                                               // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            }
            else // hemos llegado al último inodo
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                fin = !fin;
                break;
            }
        }
        if (bwrite(i, &inodos) == FAILURE)
        {
            return FAILURE;
        }
    }
    return SUCCESS;
}