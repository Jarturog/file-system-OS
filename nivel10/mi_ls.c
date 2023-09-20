// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "directorios.h"
#define TAMFILA 100
#define TAMBUFFER (TAMFILA*1000) //suponemos un máx de 1000 entradas, aunque debería ser SB.totInodos

/**
 * Programa (comando) que lista el contenido de un directorio (nombres de las entradas),
 * llamando a la función mi_dir() de la capa de directorios, que es quien construye el buffer que mostrará mi_ls.c.
 * Además indica el número total de entradas.
*/
int main(int argc, char const *argv[]){

    //Comprobación de que el número de argumentos es el correcto
    if(argc != 4){
        fprintf(stderr, ROJO "Error de sintaxis. Uso correcto: ./mi_ls <disco> </ruta_directorio> <formato>\n" RESET);
        return FALLO;
    }

    // Obtener argumentos
    const char *nombreDisco = argv[1];
    const char *camino = argv[2];
    const int formato = atoi(argv[3]); // formato 0 es simple, diferente de 0 es expandido

    // Montaje del dispositivo virtual
    if (bmount(nombreDisco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    char tipo;
    if(camino[strlen(camino)-1]=='/') { //es un directorio
        tipo = 'd';
    } else {
        tipo = 'f';
    }
    char buff[TAMBUFFER];
    memset(buff, 0, TAMBUFFER);
    int numEntradas = mi_dir(camino, buff, tipo);
    if (numEntradas < 0) {
        mostrar_error_buscar_entrada(numEntradas);
        return FALLO;
    }
    if (formato != 0 && tipo == 'd') {
        fprintf(stdout, "Total: %d\n",numEntradas);
    }
    if (numEntradas > 0) {
        if (formato == 0) {
            char *token = strtok(buff, "\n\t");
            for (int i = 0; i < numEntradas; i++) { // imprimo solo los colores y los nombres de las entradas
                char color[5];
                strncpy(color, token, 5); // guardo el color
                for (int col = 0; col < 4; col++) {
                    token = strtok(NULL, "\n\t");
                }
                fprintf(stdout, "%s%s\t", color, token); // color junto al nombre
                token = strtok(NULL, "\n\t");
            }
            fprintf(stdout, RESET "\n");
        } else {
            fprintf(stdout, "Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n--------------------------------------------------------------------------------\n%s\n", buff);
        }
    }
    
    if (bumount() == FALLO){
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}
