// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "verificacion.h"

int main(int argc, char const *argv[]){
    if(argc != 3){
        fprintf(stderr, ROJO "Sintaxis: ./verificacion <nombre_dispositivo> <directorio_simulación>\n" RESET);
        return FALLO;
    }
    
    const char *nombre = argv[1];
    const char *ruta = argv[2];

    // Montaje del dispositivo virtual
    if (bmount(nombre) == FALLO) {
        fprintf(stderr, ROJO "Error montando el dispositivo virtual\n" RESET);
        return FALLO;
    }

    int error;
    struct STAT stat_simulacion;
    error = mi_stat(ruta, &stat_simulacion);
    if (error < 0) {
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error accediendo al directorio simulación\n" RESET);
        } else {
            mostrar_error(error);
        }
        return FALLO;
    }
    
    int num_entradas = stat_simulacion.tamEnBytesLog / sizeof(struct entrada);
    if (num_entradas != NUMPROCESOS) {
        fprintf(stderr, ROJO "Deberían haber %i procesos, pero hay %i\n" RESET, NUMPROCESOS, num_entradas);
        return FALLO;
    }

    char ruta_informe[strlen(ruta) + 12];
    strcpy(ruta_informe, ruta);
    strcat(ruta_informe, "informe.txt");
    error = mi_creat(ruta_informe, 6);
    if (error < 0) {
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error creando informe.txt\n" RESET);
        } else {
            mostrar_error(error);
        }
        return FALLO;
    }

    struct entrada directorios[num_entradas];
    error = mi_read(ruta, directorios, 0, num_entradas * sizeof(struct entrada));
    if (error < 0) {
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error leyendo los directorios de los procesos\n" RESET);
        } else {
            mostrar_error(error);
        }
        return FALLO;
    }
    fprintf(stderr, "dir_sim: %s\nnumentradas: %i NUMPROCESOS: %i\n", ruta, num_entradas, NUMPROCESOS);
    // nº carácteres del output del programa
    int tam_texto = 32, tam_num = 4, tam_num_registro = 16, tam_fecha = 32, tam_pid = 16;
    int tam_buffer_escribir = num_entradas * (5 + tam_pid + tam_texto + tam_num + 4 * (tam_texto + tam_num + tam_num_registro + tam_fecha));
    char buffer_escribir[tam_buffer_escribir];
    memset(buffer_escribir, 0, tam_buffer_escribir);
    strcpy(buffer_escribir, "");

    for (unsigned int i = 0; i < num_entradas; i++) {
        struct INFORMACION info;
        int pid = atoi(1 + strchr(directorios[i].nombre, '_'));
        info.pid = pid;
        info.nEscrituras = 0;
        char str_pid[tam_pid];
        sprintf(str_pid, "%i", pid);
        char ruta_prueba[strlen(ruta) + strlen(directorios[i].nombre) + 12];
        strcpy(ruta_prueba, ruta);
        strcat(ruta_prueba, directorios[i].nombre);
        strcat(ruta_prueba, "/prueba.dat");
        
        int num_registros = BLOCKSIZE / 4;
        struct REGISTRO buffer_leer[num_registros];
        memset(buffer_leer, 0, sizeof(buffer_leer));

        // como el buffer tiene varios registros, hay que hacer un bucle para que el buffer recorra todo prueba.dat, y otro para tratar cada registro del buffer
        for (int offset_leer = 0; mi_read(ruta_prueba, buffer_leer, offset_leer, sizeof(buffer_leer)) > 0; offset_leer += sizeof(buffer_leer)) { // leer una escritura
            for (int j = 0; j < num_registros; j++) {
                if (buffer_leer[j].pid != info.pid) { // si no es válida siguiente iteración
                    continue;
                }
                if (info.nEscrituras == 0) { // Si es la primera escritura validada entonces
                    // Inicializar los registros significativos con los datos de esa escritura.
                    // ya será la de menor posición puesto que hacemos un barrido secuencial
                    info.MenorPosicion = buffer_leer[j];
                    info.MayorPosicion = buffer_leer[j];
                    info.PrimeraEscritura = buffer_leer[j];
                    info.UltimaEscritura = buffer_leer[j];
                } else { // sino comparar nº de escritura (para obtener primera y última) y actualizarlas si es preciso
                    if (buffer_leer[j].fecha.tv_usec >= info.UltimaEscritura.fecha.tv_usec && buffer_leer[j].nEscritura > info.UltimaEscritura.nEscritura)
                        info.UltimaEscritura = buffer_leer[j];
                    if (buffer_leer[j].fecha.tv_usec <= info.PrimeraEscritura.fecha.tv_usec && buffer_leer[j].nEscritura < info.PrimeraEscritura.nEscritura)
                        info.PrimeraEscritura = buffer_leer[j];
                    if (buffer_leer[j].nRegistro > info.MayorPosicion.nRegistro)
                        info.MayorPosicion = buffer_leer[j];
                    if (buffer_leer[j].nRegistro < info.MenorPosicion.nRegistro)
                        info.MenorPosicion = buffer_leer[j];
                }
                info.nEscrituras++; // Incrementar contador escrituras validadas.
            }
            memset(&buffer_leer, 0, sizeof(buffer_leer));
        }
        // Añadir la información del struct info al fichero informe.txt por el final.
        // Preparo lo que voy a escribir, que va a tener este patrón
        // PID: 10982
        // Numero de escrituras: 50
        // Primera Escritura 1	 154409	 Thu May 26 18:34:20 2022
        // Ultima Escritura	 50	 247776	 Thu May 26 18:34:22 2022
        // Menor Posición	 30	 6070	 Thu May 26 18:34:21 2022
        // Mayor Posición	 31	 479665	 Thu May 26 18:34:21 2022
        unsigned char tam_aux = 128;
        char aux[tam_aux];
        memset(aux, 0, tam_aux);
        // primeras dos líneas
        strcat(buffer_escribir, "\nPID: ");
        strcat(buffer_escribir, str_pid);
        strcat(buffer_escribir, "\nNúmero de escrituras: ");
        sprintf(aux, "%i", info.nEscrituras);
        strcat(buffer_escribir, aux);
        memset(aux, 0, tam_aux);

        struct REGISTRO registros_importantes[4];
        registros_importantes[0] = info.PrimeraEscritura;
        registros_importantes[1] = info.UltimaEscritura;
        registros_importantes[2] = info.MenorPosicion;
        registros_importantes[3] = info.MayorPosicion;
        
        char filas[4][tam_texto];
        memset(filas, 0, 4 * tam_texto);
        strcpy(filas[0], "\nPrimera Escritura\t");
        strcpy(filas[1], "Última Escritura\t");
        strcpy(filas[2], "Menor Posición\t\t");
        strcpy(filas[3], "Mayor Posición\t\t");
        // últimas 4 líneas
        for (unsigned char i = 0; i < 4; i++) {
            strcat(buffer_escribir, filas[i]);
            sprintf(aux, "%i", registros_importantes[i].nEscritura);
            strcat(buffer_escribir, aux);
            memset(aux, 0, tam_aux);
            strcat(buffer_escribir, "\t");
            sprintf(aux, "%i", registros_importantes[i].nRegistro);
            strcat(buffer_escribir, aux);
            memset(aux, 0, tam_aux);
            strcat(buffer_escribir, "\t");
            struct tm *fecha_segundos;
            fecha_segundos = localtime(&registros_importantes[i].fecha.tv_sec);
            strftime(aux, sizeof(aux), "%Y-%m-%d %H:%M:%S.", fecha_segundos);
            strcat(buffer_escribir, aux);
            memset(aux, 0, tam_aux);
            sprintf(aux, "%lu", registros_importantes[i].fecha.tv_usec);
            strcat(buffer_escribir, aux);
            strcat(buffer_escribir, "\n");
            memset(aux, 0, tam_aux);
        }
#if DEBUGN13
        fprintf(stderr, GRIS "[%i) %i escrituras validadas en %s]\n" RESET, i+1, info.nEscrituras, ruta_prueba);
#endif
    }
    strcat(buffer_escribir, "\n");
    // Se escribe en el informe
    error = mi_write(ruta_informe, &buffer_escribir, 0, strlen(buffer_escribir));
    if (error < 0) {
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error escribiendo en el informe\n" RESET);
        } else {
            mostrar_error(error);
        }
        return FALLO;
    }

    if (bumount() == FALLO) {
        fprintf(stderr, ROJO "Error desmontando el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    return EXITO;
}