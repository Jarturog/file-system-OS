// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "simulacion.h"

unsigned char acabados = 0;

void reaper() {
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0) {
        acabados++;
    }
}

int main(int argc, char const *argv[]) {
    //Asociar sigchld al reaper()
    signal(SIGCHLD, reaper);

    // Comprueba que la sintaxis sea correcta.
    if (argc != 2) {
        fprintf(stderr, ROJO "Sintaxis: ./simulacion <disco>\n" RESET);
        return FALLO;
    }

    // Variables
    const char *disco = argv[1];
    int error;
    const unsigned char TAM_FECHA = 4 + 2 + 2 + 2 + 2 + 2; // aaaammddhhmmss
    const unsigned char TAM_NOMBRE_SIMUL = TAM_FECHA + 6; // simul_aaaammddhhmmss
    const unsigned char TAM_RUTA = TAM_NOMBRE_SIMUL + 2; // /simul_aaaammddhhmmss/
    if (bmount(disco) == FALLO) {
        fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
        return FALLO;
    }

    char ruta[TAM_RUTA]; 
    char buffer_tiempo[TAM_FECHA];
    memset(ruta, 0, TAM_RUTA);
    memset(buffer_tiempo, 0, TAM_FECHA);
    time_t aux_time_t = time(NULL);
    struct tm tiempo = *localtime(&aux_time_t);
    sprintf(buffer_tiempo, "%d%02d%02d%02d%02d%02d", tiempo.tm_year + 1900, 1 + tiempo.tm_mon, tiempo.tm_mday, tiempo.tm_hour, tiempo.tm_min, tiempo.tm_sec);
    strcpy(ruta, "/");
    strcat(ruta, "simul_");
    strcat(ruta, buffer_tiempo);
    strcat(ruta, "/");

    error = mi_creat(ruta, 6);
    if (error < 0) {
        if (error == FALLO) {
            fprintf(stderr, ROJO "Error al crear el directorio de simulación\n" RESET);
        } else {
            mostrar_error_buscar_entrada(error);
        }
        return FALLO;
    }
    fprintf(stdout, "*** SIMULACIÓN DE %i PROCESOS REALIZANDO CADA UNO %i ESCRITURAS ***:\n", NUMPROCESOS, NUMESCRITURAS);
    for (unsigned char proceso = 1; proceso <= NUMPROCESOS; proceso++) {
        pid_t pid = fork();
        if (pid == 0) {
            if (bmount(disco) == FALLO) {
                fprintf(stderr, ROJO "Error de montaje del dispositivo virtual\n" RESET);
                exit(-1);
            }
            char str_pid[10]; // 10, probablemente no necesite tantos dígitos pero mejor que sobren
            memset(str_pid, 0, 10);
            sprintf(str_pid, "%d", getpid());
            unsigned char digitos_pid = strlen(str_pid);
            char ruta_hijo[TAM_RUTA + 8 + 11 + digitos_pid]; // /simul_aaaammddhhmmss/proceso_[str_pid]/prueba.dat
            memset(ruta_hijo, 0, TAM_RUTA + 8 + 11 + digitos_pid);
            strcpy(ruta_hijo, ruta);
            strcat(ruta_hijo, "proceso_");
            strcat(ruta_hijo, str_pid);
            strcat(ruta_hijo, "/");
            error = mi_creat(ruta_hijo, 6);
            if (error < 0) {
                if (error == FALLO) {
                    fprintf(stderr, ROJO "Error al crear un directorio de un proceso hijo\n" RESET);
                } else {
                    mostrar_error_buscar_entrada(error);
                }
                exit(-1);
            }
            strcat(ruta_hijo, "prueba.dat");
            error = mi_creat(ruta_hijo, 6);
            if (error < 0) {
                if (error == FALLO) {
                    fprintf(stderr, ROJO "Error al crear un prueba.dat de un proceso hijo\n" RESET);
                } else {
                    mostrar_error_buscar_entrada(error);
                }
                exit(-1);
            }
            // se consigue semilla a partir de valor aleatorio
            srand(time(NULL) + getpid());
            for (unsigned char nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) {
                struct REGISTRO registro;
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand()%REGMAX;
                error = mi_write(ruta_hijo, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                if (error < 0) {
                    if (error == FALLO) {
                        fprintf(stderr, ROJO "Error al escribir el registro %i\n" RESET, registro.nRegistro);
                    } else {
                        mostrar_error_buscar_entrada(error);
                    }
                    exit(-1);
                }
#if DEBUGSIMUL
                fprintf(stderr, GRIS "[simulación.c → Escritura %i en %s]\n" RESET, nescritura, ruta_hijo);
#endif
                usleep(50000);
            }
#if DEBUGN12
            fprintf(stderr, GRIS "[Proceso %i: Completadas %i escrituras en %s]\n" RESET, proceso, NUMESCRITURAS, ruta_hijo);
#endif
            if (bumount() == FALLO) {
                fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
                exit(-1);
            }
            exit(0);
        }
        usleep(200000);
    } 

    while (acabados < NUMPROCESOS) {
        pause();
    }

    if (bumount() == FALLO) {
        fprintf(stderr, ROJO "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}