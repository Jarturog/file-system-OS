// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"
// caché escritura y lectura
static struct ultimaEntrada ultimaEntradaEscritura[TAMCACHE], ultimaEntradaLectura[TAMCACHE];
#if !CACHELRUNOTFIFO
static char cimaPilaEscritura = 0, cimaPilaLectura = 0;
#endif
/**
 * Dada una cadena de caracteres camino (que comience por '/'), separa su contenido en dos.
 * camino: ruta sin l
 * inicial: nombre de la entrada del inodo más cerca de root de la ruta
 * final: ruta sin la parte del parámetro 'inicial'
 * tipo: devuelve f si es fichero, d si es directorio
 * Devuelve valor negativo o EXTIO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a strcpy(), strncpy(), strchr() y strlen().
*/
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    // Se comprueba que los punteros no son nulos
    if (camino == NULL || inicial == NULL || tipo == NULL || final == NULL) {
        return FALLO;
    }
    // Se comprueba que la cadena comienza con el carácter '/'
    if (*camino != '/') {
        return ERROR_CAMINO_INCORRECTO;
    }
    // Buscar el segundo '/' en la cadena
    const char *segundo_slash = strchr(camino + 1, '/');
    // Si no hay segundo '/', la cadena es el nombre del fichero
    if (segundo_slash == NULL) {
        // Copiar la cadena sin el primer '/'
        strcpy(inicial, camino + 1);
        strcpy(final, "");
        *tipo = 'f';
        return EXITO;
    }
    // Si hay segundo '/', copiar la porción entre el primer y segundo '/' en inicial
    size_t len = strlen(camino) - (strlen(segundo_slash) + 1);
    strncpy(inicial, camino + 1, len);
    // Copiamos el resto de la cadena a partir del segundo '/' en "final"
    strcpy(final, segundo_slash);
    *tipo = 'd';
    return EXITO;
}

/**
 * Busca una determinada entrada (la parte *inicial del *camino_parcial que nos devuelva extraer_camino())
 * entre las entradas del inodo correspondiente a su directorio padre (identificado con *p_inodo_dir).
 * camino_parcial: ruta en la que buscará la entrada
 * p_inodo_dir: número de inodo del directorio
 * p_inodo: número de inodo del inodo
 * p_entrada: número de entrada
 * reservar: 1 si se quiere reservar el inodo, 0 si no
 * permisos: en caso de que reservar sea 1, los permisos que tendrá el inodo a reservar
 * Devuelve valor negativo o EXTIO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a strcmp(), strlen(), extraer_camino(), leer_inodo(), mi_read_f(), reservar_inodo(), mi_write_f(), liberar_inodo(), y buscar_entrada().
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    // Variables
    struct entrada entradas[NUMENTRADASBLOQUE];
    struct inodo inodo_dir;
    char inicial[sizeof(entradas[0].nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    if (strcmp(camino_parcial, "/") == 0) { // Si es el directorio raíz
        struct superbloque SB;
        if (bread(posSB, &SB) == FALLO) return FALLO;
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }
    memset(final, 0, strlen(camino_parcial));
    memset(inicial, 0, sizeof(entradas[0].nombre));
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0) {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DEBUGBUSENT
    fprintf(stderr, GRIS "[buscar_entrada()→ inicial: %s, final: %s, reservar: %i]\n" RESET, inicial, final, reservar);
#endif
    // Buscamos la entrada cuyo nombre se encuentra en inicial
    if(leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO) return FALLO;
    if ((inodo_dir.permisos & 4) != 4) { // No tiene permisos de lectura
#if DEBUGBUSENT
    fprintf(stderr, GRIS "[buscar_entrada()→ El inodo %i no tiene permisos de lectura]\n" RESET, *p_inodo_dir);
#endif
        return ERROR_PERMISO_LECTURA;
    }
    memset(&entradas, 0, NUMENTRADASBLOQUE * sizeof(struct entrada)); // Inicializar buffer de lectura con ceros
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0; // Nº entrada inicial
    if (cant_entradas_inodo > 0) {
        if(mi_read_f(*p_inodo_dir, &entradas, 0, NUMENTRADASBLOQUE * sizeof(struct entrada)) == FALLO) return FALLO;
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(entradas[num_entrada_inodo].nombre, inicial) != 0)) {
            num_entrada_inodo++;
        }
    }
    if ((!(num_entrada_inodo < NUMENTRADASBLOQUE) || (strcmp(inicial, entradas[num_entrada_inodo].nombre) != 0)) && (num_entrada_inodo == cant_entradas_inodo)) { // La entrada no existe
        switch (reservar) {
        case 0: // Modo consulta. Como no existe retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: // Modo escritura
            // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // Si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f') {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            // Si es directorio comprobar que tiene permiso de escritura
            if ((inodo_dir.permisos & 2) != 2) {
                return ERROR_PERMISO_ESCRITURA;
            } 
            strncpy(entradas[num_entrada_inodo].nombre, inicial, TAMNOMBRE); // Copiamos el nombre del archivo en la entrada
            if (tipo == 'd' && strcmp(final, "/") != 0) { // Si es un directorio y el camino no termina aquí, cuelgan más directorios
                return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
            } 
            // Reservamos un inodo como el tipo que tiene y lo asignamos a la entrada
            entradas[num_entrada_inodo].ninodo = reservar_inodo(tipo, permisos);
            if (entradas[num_entrada_inodo].ninodo < 0) { // Si no se pudo reservar un inodo
                return FALLO; // Devolvemos el error
            }
#if DEBUGBUSENT
            fprintf(stderr, GRIS "[buscar_entrada()→ reservado inodo: %d tipo %c con permisos %d para %s]\n" RESET, entradas[num_entrada_inodo].ninodo, tipo, permisos, entradas[num_entrada_inodo].nombre);
            fprintf(stderr, GRIS "[buscar_entrada()→ creada entrada: %s, %d]\n" RESET, inicial, entradas[num_entrada_inodo].ninodo);
#endif
            // Escribimos la entrada en el directorio padre
            if (mi_write_f(*p_inodo_dir, &entradas[num_entrada_inodo], num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                if (entradas[num_entrada_inodo].ninodo != FALLO) { // Si se había reservado un inodo para la entrada
                    // Liberamos el inodo
                    liberar_inodo(entradas[num_entrada_inodo].ninodo); // no hago control de errores porque más adelante se devuelve FALLO
#if DEBUGBUSENT
                    fprintf(stderr, GRIS "[buscar_entrada()→ liberado inodo %i, reservado a %s\n" RESET, num_entrada_inodo, inicial);
#endif
                }
                return FALLO;
            }
            
        }
    }
    if (strcmp(final, "/") == 0 || strcmp(final, "") == 0) { // Si el camino termina aquí
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1)) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        } // Cortamos la recursividad
        *p_inodo = entradas[num_entrada_inodo].ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    } // Si el camino no termina aquí, seguimos buscando
    *p_inodo_dir = entradas[num_entrada_inodo].ninodo;
    return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
}

/**
 * Imprime por stderr un mensaje de error dependiendo al valor del parámetro error.
 * error: de -2 a -8 genera mensajes de error específicos, en caso contrario se imprime 'Error inesperado'.
*/
void mostrar_error_buscar_entrada(int error) {
   switch (error) {
   case ERROR_CAMINO_INCORRECTO:                        fprintf(stderr, ROJO "Error: Camino incorrecto.\n" RESET); break;
   case ERROR_PERMISO_LECTURA:                          fprintf(stderr, ROJO "Error: Permiso denegado de lectura.\n" RESET); break;
   case ERROR_NO_EXISTE_ENTRADA_CONSULTA:               fprintf(stderr, ROJO "Error: No existe el archivo o el directorio.\n" RESET); break;
   case ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO:          fprintf(stderr, ROJO "Error: No existe algún directorio intermedio.\n" RESET); break;
   case ERROR_PERMISO_ESCRITURA:                        fprintf(stderr, ROJO "Error: Permiso denegado de escritura.\n" RESET); break;
   case ERROR_ENTRADA_YA_EXISTENTE:                     fprintf(stderr, ROJO "Error: El archivo ya existe.\n" RESET); break;
   case ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO:  fprintf(stderr, ROJO "Error: No es un directorio.\n" RESET); break;
   }
}

void mostrar_error(int error) {
    if (error > ERROR_DIRECTOR_NO_VACIO) {
        return mostrar_error_buscar_entrada(error);
    }
    switch (error) {
    case ERROR_DIRECTOR_NO_VACIO:                       fprintf(stderr, ROJO "Error: El directorio no puede estar vacío.\n" RESET); break;
    case ERROR_DESTINO_TIENE_QUE_SER_DIRECTORIO:        fprintf(stderr, ROJO "Error: El destino tiene que ser un directorio.\n" RESET); break;
    }
}

/**
 * Crea un fichero/directorio y su entrada de directorio.
 * Se basa, principalmente, en la función buscar_entrada() con reservar = 1.
 * camino: ruta en la va a crear el fichero/directorio.
 * permisos: los permisos que tendrá el inodo a reservar
 * Devuelve valor negativo o EXTIO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a buscar_entrada().
*/
int mi_creat(const char *camino, unsigned char permisos) {
    // Comprobar que los permisos son válidos
    if (permisos > 7) return FALLO;
    unsigned int p_dir = 0, p_inodo = 0, p_ent = 0;
    int error = buscar_entrada(camino, &p_dir, &p_inodo, &p_ent, 1, permisos);
    if (error < 0) {
        return error;
    }
    return EXITO;
}

/**
 * Plasma en el buffer el contenido del inodo indiacdo en el camino.
 * camino: ruta en el que está la entrada
 * buffer: buffer donde estará plasmado el contenido del inodo
 * tipo: el tipo del inodo
 * Devuelve el número de entradas dentro del directorio indicado en camino o valor negativo en caso de error.
 * 
 * Internamente llama a strcat(), localtime(), mi_read_f(), leer_inodo() y buscar_entrada().
*/
int mi_dir(const char *camino, char *buffer, char tipo) {
    struct inodo inodo;
    unsigned int p_dir = 0, ninodo = 0, p_ent = 0;
    int offset = 0;

    int error = buscar_entrada(camino, &p_dir, &ninodo, &p_ent, 0, 4);
    if (error < 0) {
        return error;
    }
    if (leer_inodo(ninodo, &inodo) == FALLO){
        return FALLO;
    }
    if ((inodo.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;
    if (inodo.tipo != tipo) {
        return FALLO;
    }

    int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    
    if (tipo == 'f') { // solo tiene que imprimir el fichero
        numEntradas = 1;
        offset = p_ent * sizeof(struct entrada);
        ninodo = p_dir;
    }
    
    struct entrada entradas[numEntradas];
    
    if(mi_read_f(ninodo, &entradas, offset, numEntradas * sizeof(struct entrada)) == FALLO){
        return FALLO;
    }

    for (int leidos = 0; leidos < numEntradas; leidos++) {

        if(leer_inodo(entradas[leidos].ninodo, &inodo) == FALLO) return FALLO;

        if (inodo.tipo == 'd') {
            strcat(buffer, AMARILLO);
            strcat(buffer, "d");
        } else {
            strcat(buffer, MAGENTA);
            strcat(buffer, "f");
        }
        
        strcat(buffer, "\t");

        if ((inodo.permisos & 4) == 4) {
            strcat(buffer, "r");
        } else {
            strcat(buffer, "-");
        }
        if ((inodo.permisos & 2) == 2) {
            strcat(buffer, "w");
        } else {
            strcat(buffer, "-");
        }
        if ((inodo.permisos & 1) == 1) {
            strcat(buffer, "x");
        } else {
            strcat(buffer, "-");
        }

        struct tm *tm;
        tm = localtime(&inodo.mtime);
        char bufferTime[128];
        sprintf(bufferTime, "\t%d-%02d-%02d %02d:%02d:%02d\t%i", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, inodo.tamEnBytesLog);
        strcat(buffer, bufferTime);
        strcat(buffer, "\t");
        strcat(buffer, entradas[leidos].nombre);
        strcat(buffer, "\n");
    }
    strcat(buffer, RESET);

    return numEntradas;
}

/**
 * Busca la entrada *camino con buscar_entrada() para obtener el nº de inodo (p_inodo).
 * Cambia los permisos del inodo en el camino.
 * camino: ruta de la entrada
 * permisos: los permisos que tendrá el inodo
 * Devuelve valor negativo o EXTIO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a mi_chmod_f() y buscar_entrada().
*/
int mi_chmod(const char *camino, unsigned char permisos) {
    // Buscar entrada de camino
    unsigned int p_dir = 0, p_inodo = 0, p_ent = 0;
    int error = buscar_entrada(camino, &p_dir, &p_inodo, &p_ent, 0, permisos);
    if (error < 0) {
        return error; // No hace falta print porque buscar entrada ya devuelve el error
    }

    // Cambiar permisos del inodo
    if (mi_chmod_f(p_inodo, permisos) == FALLO) {
        return FALLO; 
    }

    return EXITO; 
}

/**
 * Busca la entrada *camino con buscar_entrada() para obtener el p_inodo.
 * Crea un STAT con la información del inodo en el camino.
 * camino: ruta de la entrada
 * p_stat: struct STAT en el que se almacenará el STAT conseguido del inodo en camino
 * Devuelve el número de inodo o valor negativo en caso de error.
 * 
 * Internamente llama a mi_stat_f() y buscar_entrada().
*/
int mi_stat(const char *camino, struct STAT *p_stat) {
    unsigned int p_dir = 0, p_inodo = 0, p_ent = 0;
    int error = buscar_entrada(camino, &p_dir, &p_inodo, &p_ent, 0, p_stat->permisos);
    if (error < 0) {
        return error;
    }
    if (mi_stat_f(p_inodo, p_stat) == FALLO) {
        return FALLO;
    }
    return p_inodo;
}

/**
 * Escribe contenido en un fichero.
 * camino: ruta en la que se encuentra el fichero
 * buf: contenido que se va a escribir
 * offset: desplazamiento en bytes
 * nbytes: tamaño de buf en bytes
 * Devuelve valor negativo o la cantidad de bytes escritos dependiendo de si ha habido error o no.
 * 
 * Internamente llama a mi_write_f(), strcmp(), strcpy(), gettimeofday(), y buscar_entrada().
*/
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_dir = 0, p_inodo = 0, p_ent = 0;
    int encontrado = 0; // encontrado = false
    int hueco = -1; // posición vacía
#if CACHELRUNOTFIFO
    int entradaAntigua = -1; // entrada más antigua
    struct timeval instanteActual;
    if(gettimeofday(&instanteActual, NULL) < 0){
        return FALLO;
    }
    long fechaMasAntigua = instanteActual.tv_usec;
#endif
    for (int i = 0; i < TAMCACHE; i++) {
        if (ultimaEntradaEscritura[i].camino != NULL && strcmp(ultimaEntradaEscritura[i].camino, camino) == 0) { // si hay entradas y la entrada ya ha sido llamada
            encontrado = !encontrado;
            hueco = i;
            p_inodo = ultimaEntradaEscritura[hueco].p_inodo;
#if CACHELRUNOTFIFO
            ultimaEntradaEscritura[hueco].momento_insertada = instanteActual.tv_usec;
#endif
            break;
        }
#if CACHELRUNOTFIFO
        // si no se ha encontrado posición vacía
        if(ultimaEntradaEscritura[i].camino == NULL) { // si se ha encontrado
            hueco = i; // me guardo la posición vacía para luego no tener que volver a recorrer la caché
        } else if (ultimaEntradaEscritura[i].camino != NULL && ultimaEntradaEscritura[i].momento_insertada < fechaMasAntigua) {  // mientras no se encuentre el hueco se consigue la fecha más antigua
            fechaMasAntigua = ultimaEntradaEscritura[i].momento_insertada;
            entradaAntigua = i;
        }
#endif
    }

#if DEBUGN9
    if (encontrado) {
        fprintf(stderr, GRIS "[mi_write() → Utilizamos caché[%d]: %s\n" RESET, hueco, camino);
    }
#endif

    if (!encontrado) { // si no estaba en la caché
        int error = buscar_entrada(camino, &p_dir, &p_inodo, &p_ent, 0, 4); // permisos de lectura
        if (error < 0) {
            return error;
        }
#if CACHELRUNOTFIFO
        // se añade a la caché
        if (entradaAntigua >= 0) { // se compruebe que hubiese alguna más antigua, porque en caso contrario se añade por el principio
            hueco = entradaAntigua; // se insertará donde estaba la más antigua
        } else {
            hueco = 0;
        }
        ultimaEntradaEscritura[hueco].momento_insertada = instanteActual.tv_usec;
#else
        hueco = (cimaPilaEscritura++)%TAMCACHE;
#endif
        strcpy(ultimaEntradaEscritura[hueco].camino, camino);
        ultimaEntradaEscritura[hueco].p_inodo = p_inodo;
#if DEBUGN9
        fprintf(stderr, GRIS "[mi_write() → Reemplazamos la caché[%d]: %s]\n" RESET, hueco, camino);
#endif
    }

    return mi_write_f(p_inodo, buf, offset, nbytes);
}

/**
 * Lee los nbytes del fichero indicado por camino, a partir del offset pasado por parámetro y copiarlos en el buffer buf.
 * camino: ruta en la que se encuentra el fichero
 * buf: buffer en el que se guardará el contenido del fichero
 * offset: desplazamiento en bytes
 * nbytes: tamaño de buf en bytes
 * Devuelve valor negativo o la cantidad de bytes leídos dependiendo de si ha habido error o no.
 * 
 * Internamente llama a mi_read_f(), strcmp(), strcpy(), gettimeofday(), y buscar_entrada().
*/
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes){
    unsigned int p_dir = 0, p_inodo = 0, p_ent = 0;
    int encontrado = 0; // encontrado = false
    int hueco = -1; // posición vacía
#if CACHELRUNOTFIFO
    int entradaAntigua = -1; // entrada más antigua
    struct timeval instanteActual;
    if(gettimeofday(&instanteActual, NULL) < 0){
        return FALLO;
    }
    long fechaMasAntigua = instanteActual.tv_usec;
#endif
    for (int i = 0; i < TAMCACHE; i++) {
        if (ultimaEntradaLectura[i].camino != NULL && strcmp(ultimaEntradaLectura[i].camino, camino) == 0) { // si hay entradas y la entrada ya ha sido llamada
            encontrado = !encontrado;
            hueco = i;
            p_inodo = ultimaEntradaLectura[hueco].p_inodo;
#if CACHELRUNOTFIFO
            ultimaEntradaLectura[hueco].momento_insertada = instanteActual.tv_usec;
#endif
            break;
        }
#if CACHELRUNOTFIFO
        // si no se ha encontrado posición vacía
        if(ultimaEntradaLectura[i].camino == NULL) { // si se ha encontrado
            hueco = i; // me guardo la posición vacía para luego no tener que volver a recorrer la caché
        } else if (ultimaEntradaLectura[i].camino != NULL && ultimaEntradaLectura[i].momento_insertada < fechaMasAntigua) {  // mientras no se encuentre el hueco se consigue la fecha más antigua
            fechaMasAntigua = ultimaEntradaLectura[i].momento_insertada;
            entradaAntigua = i;
        }
#endif
    }

#if DEBUGN9
    if (encontrado) {
        fprintf(stderr, GRIS "[mi_read() → Utilizamos caché[%d]: %s\n" RESET, hueco, camino);
    }
#endif

    if (!encontrado) { // si no estaba en la caché
        int error = buscar_entrada(camino, &p_dir, &p_inodo, &p_ent, 0, 4); // permisos de lectura
        if (error < 0) {
            return error;
        }
#if CACHELRUNOTFIFO
        // se añade a la caché
        if (entradaAntigua >= 0) { // se compruebe que hubiese alguna más antigua, porque en caso contrario se añade por el principio
            hueco = entradaAntigua; // se insertará donde estaba la más antigua
        } else {
            hueco = 0;
        }
        ultimaEntradaLectura[hueco].momento_insertada = instanteActual.tv_usec;
#else
        hueco = (cimaPilaLectura++)%TAMCACHE;
#endif
        strcpy(ultimaEntradaLectura[hueco].camino, camino);
        ultimaEntradaLectura[hueco].p_inodo = p_inodo;
        
#if DEBUGN9
        fprintf(stderr, GRIS "[mi_read() → Reemplazamos la caché[%d]: %s]\n" RESET, hueco, camino);
#endif
    }

    return mi_read_f(p_inodo, buf, offset, nbytes);
}

/**
 * Enlaza la entrada de directorio camino2 al mismo inodo asociado a otra entrada de directorio camino1.
 * Primero busca la entrada camino1 y comprueba que tenga permiso de lectura, luego busca o crea la entrada 
 * camino2 con permisos de escritura, y si la entrada se crea correctamente, lee la entrada creada correspondiente 
 * a camino2 y la modifica para asociarle el mismo inodo que el asociado a camino1. Luego escribe la entrada modificada
 * en el directorio correspondiente, libera el inodo que se ha asociado a la entrada creada y actualiza el numero de 
 * enlaces y ctime en el inodo asociado a camino1.
 * camino1: ruta en la que se encuentra el fichero del cual se va a crear el enlace
 * camino2: ruta en la que estará el enlace del fichero
 * Devuelve valor negativo o EXITO dependiendo de si ha habido error o no.
 * 
 * Internamente llama a mi_write_f(), mi_read_f(), leer_inodo(), liberar_inodo(), escribir_inodo(), strcmp(), strcpy(), strlen(), time(), y buscar_entrada().
*/
int mi_link(const char *camino1, const char *camino2) {
    unsigned int p_inodo_dir1 = 0, p_inodo1 = 0, p_entrada1 = 0, p_inodo_dir2 = 0, p_inodo2 = 0, p_entrada2 = 0;

    if (!camino1 || !camino2) {
        return FALLO;
    }
    
    if (camino1[strlen(camino1) - 1] == '/' || camino2[strlen(camino2) - 1] == '/') { // Comprobar que las rutas no apunten a ficheros
        return FALLO;
    }

    int error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4);
    if  (error < 0) { // Buscar entrada de camino1
        return error;
    }
    error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);
    if (error < 0) {  // Crear entrada para camino2
        return error;
    }
   
    struct inodo inodo1;
    if (leer_inodo(p_inodo1, &inodo1) == FALLO) {
        return FALLO;
    }

    struct entrada entrada; // Leemos la entrada creada correspondiente a camino2, o sea la entrada p_entrada2 de p_inodo_dir2.
    if (mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) {
        return FALLO;
    }

    entrada.ninodo = p_inodo1; // Creamos el enlace: Asociamos a esta entrada el mismo inodo que el asociado a la entrada de camino1, es decir p_inodo1.
    
    if (mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) { // Escribimos la entrada modificada en p_inodo_dir2.
        return FALLO;
    }

    if (liberar_inodo(p_inodo2) == FALLO) { // Liberamos el inodo que se ha asociado a la entrada creada, p_inodo2. 
        return FALLO;
    }

    inodo1.nlinks++;  // Incrementamos la cantidad de enlaces (nlinks) de p_inodo1, actualizamos el ctime y lo salvamos.
    inodo1.ctime = time(NULL);

    if (escribir_inodo(p_inodo1, &inodo1) == FALLO) { // guardar el inodo
        return FALLO;
    }

    return EXITO;
}

/**
 * Borra la entrada de directorio especificada y en caso de que fuera el último enlace existente, borrar el propio fichero/directorio.
 * Es decir que esta función nos servirá tanto para borrar un enlace a un fichero como para eliminar un fichero o directorio que no contenga enlaces.
 * camino: ruta del enlace/fichero/directorio a eliminar.
 * 
 * Internamente llama a mi_write_f(), mi_read_f(), leer_inodo(), liberar_inodo(), mi_truncar_f(), escribir_inodo(), time(), y buscar_entrada().
*/
int mi_unlink(const char *camino) {
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

    if (!camino) { // si NULL
        return  FALLO;
    }

    int error;
    if  ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0) { // Buscar entrada de camino
        return error;
    }
    // Mediante la función leer_inodo() leemos el inodo asociado al directorio que contiene la entrada que queremos eliminar (p_inodo_dir)
    struct inodo inodo, directorio;
    if (leer_inodo(p_inodo, &inodo) == FALLO || leer_inodo(p_inodo_dir, &directorio) == FALLO) {
        return FALLO;
    }
    // Si se trata de un directorio y no está vacío (inodo.tamEnBytesLog > 0) entonces no se puede borrar y salimos de la función
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0) {
        return ERROR_DIRECTOR_NO_VACIO;
    }
    // obtenemos el nº de entradas que tiene (inodo_dir.tamEnBytesLog/sizeof(struct entrada))
    int nEntrada = (directorio.tamEnBytesLog / sizeof(struct entrada)) - 1;
    // Si no es la última entrada, entonces tenemos que leer la última y escribirla en la posición de la entrada
    // que queremos eliminar (p_entrada), y después ya podemos truncar el inodo como en el caso anterior.
    // De esta manera siempre dejaremos las entradas de un directorio consecutivas (sin huecos) para cuando tengamos que utilizar la función buscar_entrada().
    if (p_entrada != nEntrada) {
        struct entrada ent;
        if (mi_read_f(p_inodo_dir, &ent, nEntrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO ||
            mi_write_f(p_inodo_dir, &ent, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) {
            return FALLO;
        }
    }

    if (mi_truncar_f(p_inodo_dir, nEntrada * sizeof(struct entrada)) == FALLO) {
        return FALLO;
    }

    inodo.nlinks--; // directorio.nlinks--?
    if (inodo.nlinks == 0) { // Si no quedan enlaces (nlinks=0) entonces liberaremos el inodo
        if (liberar_inodo(p_inodo) == FALLO) {
            return FALLO;
        }
    } else { // en caso contrario actualizamos su ctime y escribimos el inodo.
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, &inodo) == FALLO) {
            return FALLO;
        }
    }

    return EXITO;
}

/**
* Renombrar un archivo/directorio
* Utiliza buscar_entrada() para encontrar la entrada correspondiente al archivo o directorio antiguo en la ruta especificada por *camino
* Se lee el inodo correspondiente al fichero o directorio antiguo para ver si es fichero/directorio
* Si es archivo, se cambia el nombre de la entrada correspondiente a ese archivo al nuevo nombre especificado 
* Si es directorio, se busca una entrada correspondiente al nuevo nombre en el mismo directorio 
*   -Si se encuentra, se devuelve el error con mostrar_error_buscar_entrada()
*   -Si no se encuentra, se cambia el nombre de la entrada correspondiente al directorio antiguo al nuevo nombre especificado
* *camino: ruta del archivo/directorio a renombrar
* *nombre_nuevo: nuevo nombre
* Devuelve 0 (EXITO) si todo ha ido bien, -1 (FALLO) si ha habido error
* Internamente se llama a buscar_entrada(), mi_read_f() y mi_write_f()
*/
int mi_rn(const char *camino, const char *nombre_nuevo) {

    unsigned int p_inodo_dir = 0, p_inodo_ant = 0, p_entrada_ant = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo_ant, &p_entrada_ant, 0, 0);
    if (error < 0) { // Busca entrada correspondiente al fichero/directorio antiguo
        return error;
    }
    char camino_nuevo[strlen(camino) + strlen(nombre_nuevo)];
    memset(camino_nuevo, 0, strlen(camino) + strlen(nombre_nuevo));
    char camino_aux[strlen(camino)];
    memset(camino_aux, 0, strlen(camino));
    char str_anterior[strlen(camino)];
    memset(str_anterior, 0, strlen(camino));
    strcpy(camino_aux, camino);
    char *token = strtok(camino_aux, "/");
    strcpy(str_anterior, "");
    while (token != NULL) {
        strcat(camino_nuevo, str_anterior);
        strcpy(str_anterior, "/");
        strcat(str_anterior, token);
        token = strtok(NULL, "/");
    }
    strcat(camino_nuevo, "/");
    strcat(camino_nuevo, nombre_nuevo);
    if (camino[strlen(camino)-1] == '/') { // si era un directorio
        strcat(camino_nuevo, "/");
    }
    
    unsigned int p_inodo_dir_repe = 0, p_inodo_repe = 0, p_entrada_repe = 0;
    if (buscar_entrada(camino_nuevo, &p_inodo_dir_repe, &p_inodo_repe, &p_entrada_repe, 0, 0) >= 0) { // Busca entrada correspondiente al fichero/directorio repetido en caso de que haya
        return ERROR_ENTRADA_YA_EXISTENTE; // si ya existe un inodo con el nombre nuevo error
    }

    struct entrada entrada;
    if (mi_read_f(p_inodo_dir, &entrada, p_entrada_ant * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
        return FALLO;
    }

    strcpy(entrada.nombre, nombre_nuevo); // Cambiamos el nombre de la entrada antigua
    if (mi_write_f(p_inodo_dir, &entrada, p_entrada_ant * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
        return FALLO;
    }
    return EXITO;
}

/**
* Mueve un un fichero o directorio a otro directorio
* Crea la entrada nombre (puede ser de fichero o directorio) en /destino/ y borra la entrada nombre en el directorio /origen/. 
* Hay que comprobar que exista /destino/ y que sea el nombre de un directorio (acabado en ‘/’).
* Hay que comprobar que no existe ya  /destino/nombre.
* Esta operación sólo afectará al nombre de la entrada correspondiente; el inodo del fichero o directorio movido sigue siendo el mismo.
*/
int mi_mv(const char *camino_origen, const char *camino_destino) {
    int error = mi_cp(camino_origen, camino_destino);
    if (error < 0) {
        return error; 
    }
    return mi_rm_r(camino_origen);
}

/**
* Copia un fichero o directorio en otro directorio
*/
int mi_cp(const char *camino_origen, const char *camino_destino) {
    
    if (camino_destino[strlen(camino_destino) - 1] != '/') { // si es un directorio
        return ERROR_DESTINO_TIENE_QUE_SER_DIRECTORIO;
    }
    // conseguimos el nombre del fichero que se va a mover y el camino que formaría: camino_destino + nombre
    char nombre_movido[strlen(camino_origen)];
    memset(nombre_movido, 0, strlen(camino_origen));
    char camino_aux[strlen(camino_origen)];
    memset(camino_aux, 0, strlen(camino_origen));
    strcpy(camino_aux, camino_origen);
    char *token = strtok(camino_aux, "/");
    while (token != NULL) {
        strcpy(nombre_movido, token);
        token = strtok(NULL, "/");
    }
    char camino_destino_y_nombre[strlen(camino_destino)+strlen(nombre_movido)+2];
    memset(camino_destino_y_nombre, 0, strlen(camino_destino)+strlen(nombre_movido)+2);
    strcpy(camino_destino_y_nombre, camino_destino);
    strcat(camino_destino_y_nombre, nombre_movido);
    if (camino_origen[strlen(camino_origen) - 1] == '/') { // si es un directorio
        strcat(camino_destino_y_nombre, "/");
    }
    // se cogen los datos del inodo que se va a mover
    unsigned int p_inodo_dir_origen = 0, p_inodo_movido = 0, p_entrada_movida = 0;
    int error = buscar_entrada(camino_origen, &p_inodo_dir_origen, &p_inodo_movido, &p_entrada_movida, 0, 0);
    if (error < 0) {
        return error;
    }
    // leo el inodo
    struct inodo inodo_movido;
    if (leer_inodo(p_inodo_movido, &inodo_movido) < 0) {
        return FALLO;
    }
    // se reserva inodo y crea entrada
    unsigned int p_inodo_dir_nuevo = 0, p_inodo_nuevo = 0, p_entrada_nueva = 0;
    error = buscar_entrada(camino_destino_y_nombre, &p_inodo_dir_nuevo, &p_inodo_nuevo, &p_entrada_nueva, 1, inodo_movido.permisos);
    if (error < 0) {
        return error;
    }
    // si es fichero se copia el contenido y aquí acaba
    if (inodo_movido.tipo == 'f') {
        struct inodo i; // ------------------------------------------------------------------------------------------------
        
        const unsigned int tambuffer = BLOCKSIZE;
        unsigned int offset = 0;
        char buffer[tambuffer];
        memset(buffer, 0, tambuffer);
        char bufferCeros[tambuffer];
        memset(bufferCeros, 0, tambuffer);
        int leidos = mi_read(camino_origen, buffer, offset, tambuffer);
        while (leidos > 0){ // fin cuando leidos = 0
            if (memcmp(buffer, bufferCeros, leidos) != 0) {
                error = mi_write(camino_destino_y_nombre, buffer, offset, leidos);
                if (error < 0) {
                    return error;
                }
                memset(buffer, 0, tambuffer);
            }
            offset += tambuffer;
            leidos = mi_read(camino_origen, buffer, offset, tambuffer);
            leer_inodo(p_inodo_nuevo, &i); // ------------------------------------------------------------------------------------------------
        }
        if (leidos < 0) {
            mostrar_error_buscar_entrada(leidos);
            return leidos;
        }
        return EXITO;
    }
    // si es directorio se tratan las entradas que cuelguen de él
    int numEntradas = inodo_movido.tamEnBytesLog / sizeof(struct entrada);
    struct entrada entradas[numEntradas];
    
    if(mi_read_f(p_inodo_movido, &entradas,0, numEntradas * sizeof(struct entrada)) == FALLO){
        return FALLO;
    }

    for (int leidos = 0; leidos < numEntradas; leidos++) {
        char camino_inodo[strlen(camino_origen) + 1 + strlen(entradas[leidos].nombre)];
        memset(camino_inodo, 0, strlen(camino_origen) + 1 + strlen(entradas[leidos].nombre));
        strcpy(camino_inodo, camino_origen);
        strcat(camino_inodo, entradas[leidos].nombre);
        struct inodo inodo;
        if (leer_inodo(entradas[leidos].ninodo, &inodo) < 0) {
            return FALLO;
        }
        if (inodo.tipo == 'd') {
            strcat(camino_inodo, "/");
        }
        if ((error = mi_cp(camino_inodo, camino_destino_y_nombre)) < 0) {
            return error;
        }
    }

    return EXITO;
}

/**
* Borra recursivamente un directorio no vacío
*/
int mi_rm_r(const char *camino){
    int error;
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;

    if (camino[strlen(camino) - 1] != '/') {
        if ((error = mi_unlink(camino)) < 0) {
            return error;
        }
        return EXITO; // si no es un directorio
    }

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0)) < 0) {
        return error;
    }
    struct inodo inodo;
    if(leer_inodo(p_inodo, &inodo) == FALLO) return FALLO;

    int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    struct entrada entradas[numEntradas];
    
    if(mi_read_f(p_inodo, &entradas,0, numEntradas * sizeof(struct entrada)) == FALLO){
        return FALLO;
    }

    for (int leidos = 0; leidos < numEntradas; leidos++) {

        char camino_inodo[strlen(camino) + 1 + strlen(entradas[leidos].nombre)];
        memset(camino_inodo, 0, strlen(camino) + 1 + strlen(entradas[leidos].nombre));
        strcpy(camino_inodo, camino);
        strcat(camino_inodo, entradas[leidos].nombre);
        struct inodo inodo;
        if (leer_inodo(entradas[leidos].ninodo, &inodo) < 0) {
            return FALLO;
        }
        if (inodo.tipo == 'd') {
            strcat(camino_inodo, "/");
        }
        if ((error = mi_rm_r(camino_inodo)) < 0) {
            return error;
        }
    }

    if ((error = mi_unlink(camino)) < 0) {
        return error;
    }

    return EXITO;
}
