// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "ficheros.h"

#define TAMNOMBRE 60 //tamaño del nombre de directorio o fichero, en Ext2 = 256
#define NUMENTRADASBLOQUE (BLOCKSIZE / sizeof(struct entrada)) // Bloques 1024B/64B entrada = 16 entradas en un bloque
#define PROFUNDIDAD 32 //profundidad máxima del árbol de directorios
#define TAMCACHE 3 // tamaño del caché de ultimaEntrada para escritura y lectura
struct entrada {
    char nombre[TAMNOMBRE];
    unsigned int ninodo;
};
struct ultimaEntrada{
    char camino [TAMNOMBRE*PROFUNDIDAD];
    int p_inodo;
#if CACHELRUNOTFIFO
    time_t momento_insertada;
#endif
};


// nivel 7
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo);
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos);
void mostrar_error_buscar_entrada(int error);
void mostrar_error(int error);
// nivel 8
int mi_creat(const char *camino, unsigned char permisos);
int mi_dir(const char *camino, char *buffer, char tipo);
int mi_chmod(const char *camino, unsigned char permisos);
int mi_stat(const char *camino, struct STAT *p_stat);
// nivel 9
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes);
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes);
// nivel 10
int mi_link(const char *camino1, const char *camino2);
int mi_unlink(const char *camino);
// funcionalidades extra nivel 10
int mi_rn(const char *camino, const char *nombre_nuevo);
int mi_mv(const char *camino_origen, const char *camino_destino);
int mi_cp(const char *camino_origen, const char *camino_destino);
int mi_rm_r(const char *camino);

// errores buscar_entrada()
#define ERROR_CAMINO_INCORRECTO -2
#define ERROR_PERMISO_LECTURA -3
#define ERROR_NO_EXISTE_ENTRADA_CONSULTA -4
#define ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO -5
#define ERROR_PERMISO_ESCRITURA -6
#define ERROR_ENTRADA_YA_EXISTENTE -7
#define ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO -8
// resto de errores
#define ERROR_DIRECTOR_NO_VACIO -9
#define ERROR_DESTINO_TIENE_QUE_SER_DIRECTORIO -10
