// Autores:  Juan Arturo Abaurrea Calafell y Marta González Juan
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
#if DEBUGN7
    fprintf(stderr, GRIS "[buscar_entrada()→ inicial: %s, final: %s, reservar: %i]\n" RESET, inicial, final, reservar);
#endif
    // Buscamos la entrada cuyo nombre se encuentra en inicial
    if(leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO) return FALLO;
    if ((inodo_dir.permisos & 4) != 4) { // No tiene permisos de lectura
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
#if DEBUGN7
                fprintf(stderr, GRIS "[buscar_entrada()→ reservado inodo: %d tipo %c con permisos %d para %s]\n" RESET, entradas[num_entrada_inodo].ninodo, tipo, permisos, entradas[num_entrada_inodo].nombre);
#endif
            } else { // Si es un fichero
                // Reservamos un inodo como fichero y lo asignamos a la entrada
                entradas[num_entrada_inodo].ninodo = reservar_inodo('f', permisos);
                if (entradas[num_entrada_inodo].ninodo < 0) { // Si no se pudo reservar un inodo
                    return FALLO; // Devolvemos el error
                }
#if DEBUGN7
                fprintf(stderr, GRIS "[buscar_entrada()→ reservado inodo: %d tipo %c con permisos %d para %s]\n" RESET, entradas[num_entrada_inodo].ninodo, tipo, permisos, entradas[num_entrada_inodo].nombre);
#endif
            }
#if DEBUGN7
            fprintf(stderr, GRIS "[buscar_entrada()→ creada entrada: %s, %d] \n" RESET, inicial, entradas[num_entrada_inodo].ninodo);
#endif
            // Escribimos la entrada en el directorio padre
            if (mi_write_f(*p_inodo_dir, &entradas[num_entrada_inodo], num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0) {
                if (entradas[num_entrada_inodo].ninodo != FALLO) { // Si se había reservado un inodo para la entrada
                    // Liberamos el inodo
                    liberar_inodo(entradas[num_entrada_inodo].ninodo); // no hago control de errores porque más adelante se devuelve FALLO
#if DEBUGN7
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

// Función que recibe el código de error, de la función buscar_entrada(), e imprime el tipo de error que se ha producido
void mostrar_error_buscar_entrada(int error) {
    fprintf(stderr, ROJO "Error: " RESET);
    switch (error) {
    case -2:
        fprintf(stderr, ROJO "Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, ROJO "Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, ROJO "No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, ROJO "No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, ROJO "Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        fprintf(stderr, ROJO "El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, ROJO "No es un directorio.\n" RESET);
        break;
    }
}