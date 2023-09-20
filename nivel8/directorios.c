// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    // Se comprueba que los punteros no son nulos
    if (camino == NULL || inicial == NULL || tipo == NULL || final == NULL) {
        return FALLO;
    }
    // Se comprueba que la cadena comienza con el carácter '/'
    if (*camino != '/') {
        return ERROR_CAMINO_INCORRECTO;
    }
    // Se copia el primer carácter de la cadena en tipo
    strcpy(tipo, "d");
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
    if (len >= TAMNOMBRE) return FALLO;
    strncpy(inicial, camino + 1, len);
    // Copiamos el resto de la cadena a partir del segundo '/' en "final"
    strcpy(final, segundo_slash);
    // Buscamos la entrada cuyo nombre se encuentra en inicial
    // Comprobamos que "final" no termina con el carácter '/', en caso contrario, el valor de tipo debe ser 'd'
    size_t final_len = strlen(final);
    
    if (final_len > 0 && final[final_len - 1] == '/') {
        *tipo = 'd';
    } else {
        *tipo = 'f';
    }
    return EXITO;
}

int buscar_entrada(const char * camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
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
    if ((strcmp(inicial, entradas[num_entrada_inodo].nombre) != 0) && (num_entrada_inodo == cant_entradas_inodo)) { // La entrada no existe
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
            if (tipo == 'd') { // Si es un directorio
                if (strcmp(final, "/") != 0) { // Si el camino no termina aquí, cuelgan más directorios
                    return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                } // Si el camino termina aquí
                // Reservamos un inodo como directorio y lo asignamos a la entrada
                entradas[num_entrada_inodo].ninodo = reservar_inodo('d', permisos);
                if (entradas[num_entrada_inodo].ninodo < 0) { // Si no se pudo reservar un inodo
                    return FALLO;
                }
#if DEBUGBUSENT
                fprintf(stderr, GRIS "[buscar_entrada()→ reservado inodo: %d tipo %c con permisos %d para %s]\n" RESET, entradas[num_entrada_inodo].ninodo, tipo, permisos, entradas[num_entrada_inodo].nombre);
#endif
            } else { // Si es un fichero
                // Reservamos un inodo como fichero y lo asignamos a la entrada
                entradas[num_entrada_inodo].ninodo = reservar_inodo('f', permisos);
                if (entradas[num_entrada_inodo].ninodo < 0) { // Si no se pudo reservar un inodo
                    return FALLO; // Devolvemos el error
                }
#if DEBUGBUSENT
                fprintf(stderr, GRIS "[buscar_entrada()→ reservado inodo: %d tipo %c con permisos %d para %s]\n" RESET, entradas[num_entrada_inodo].ninodo, tipo, permisos, entradas[num_entrada_inodo].nombre);
#endif
            }
#if DEBUGBUSENT
            fprintf(stderr, GRIS "[buscar_entrada()→ creada entrada: %s, %d] \n" RESET, inicial, entradas[num_entrada_inodo].ninodo);
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

void mostrar_error_buscar_entrada(int error) {
   switch (error) {
   case -2: fprintf(stderr, ROJO "Error: Camino incorrecto.\n" RESET); break;
   case -3: fprintf(stderr, ROJO "Error: Permiso denegado de lectura.\n" RESET); break;
   case -4: fprintf(stderr, ROJO "Error: No existe el archivo o el directorio.\n" RESET); break;
   case -5: fprintf(stderr, ROJO "Error: No existe algún directorio intermedio.\n" RESET); break;
   case -6: fprintf(stderr, ROJO "Error: Permiso denegado de escritura.\n" RESET); break;
   case -7: fprintf(stderr, ROJO "Error: El archivo ya existe.\n" RESET); break;
   case -8: fprintf(stderr, ROJO "Error: No es un directorio.\n" RESET); break;
   default: fprintf(stderr, ROJO "Error inesperado.\n" RESET);
   }
}

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

int mi_dir(const char *camino, char *buffer, char tipo) {
    struct inodo inodo;
    unsigned int p_dir = 0, ninodo = 0, p_ent = 0;
    
    int error = buscar_entrada(camino, &p_dir, &ninodo, &p_ent, 0, 4);
    if (error < 0) {
        return error;
    }
    if(leer_inodo(ninodo, &inodo) == FALLO){
        return FALLO;
    }
    if ((inodo.permisos & 4) != 4) return ERROR_PERMISO_LECTURA;
    if (inodo.tipo != tipo) {
        fprintf(stderr, ROJO "Error: la sintaxis no concuerda con el tipo" RESET);
        return FALLO;
    }

    int numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    struct entrada entradas[numEntradas];
    
    if(mi_read_f(ninodo, &entradas, 0, numEntradas * sizeof(struct entrada)) == FALLO){
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

int mi_chmod(const char *camino, unsigned char permisos) {
    // Buscar entrada de camino
    unsigned int p_dir = 0, p_inodo = 0, p_ent = 0;
    int error = buscar_entrada(camino, &p_dir, &p_inodo, &p_ent, 0, permisos);
    if (error < 0) {
        return error; // No hace falta print porque buscar entrada ya devuelve el error
    }

    // Cambiar permisos del inodo
    if (mi_chmod_f(p_inodo, permisos) == FALLO) {
        fprintf(stderr, ROJO "Error, no se han podido cambiar los permisos\n" RESET);
        return FALLO; 
    }

    return EXITO; 
}

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
