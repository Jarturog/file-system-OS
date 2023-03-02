// inicializar el disco con 10000 bloques inicializados a 0
// bloques físicos =/ bloques lógicos
// 1 mediante argc me dicen nombre dispositivos y cantidad de bloques y si está mal escrito aviso de buena sintaxis
// 2 montar dispositivo (bmount)
// 3 inicializar a 0s (bucle de llamadas a bwrite)
// 4 desmontar dispositivo (bmount)

#include "bloques.h" // solamente para la primera sesión

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis errónea: ./mi_fks <nombre del fichero> <numero de bloques>\n");
        return FAILURE;
    }

    char *camino = argv[1];
    int nbloques = atoi(argv[2]);
    unsigned char buf[BLOCKSIZE];
    if (memset(buf, 0, BLOCKSIZE) <= 0) // si error
    {
        return FAILURE;
    }

    if (bmount(camino) == FAILURE)
    {
        fprintf(stderr, "Error\n");
        return FAILURE;
    }

    for (int i = 0; i < nbloques; i++)
    {
        if (bwrite(i, buf) == FAILURE)
        {
            fprintf(stderr, "Error\n");
            return FAILURE;
        }
    }

    if (bumount() == FAILURE)
    {
        fprintf(stderr, "Error\n");
        return FAILURE;
    }
    return SUCCESS;
}