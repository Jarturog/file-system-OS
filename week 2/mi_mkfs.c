#include <ficheros_basico.h>

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
    tamAI(nbloques/4);
    initSB();
    initMB();
    initAI();

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