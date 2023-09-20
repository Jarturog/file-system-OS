// Autores: Juan Arturo Abaurrea Calafell y Marta González Juan
#include "simulacion.h"


struct INFORMACION {
    int pid;
    unsigned int nEscrituras;
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};
