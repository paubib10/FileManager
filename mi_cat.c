#include "directorios.h"

#define TAMBUFFER 1024

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }

    char *camino = argv[2];

    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el disco\n");
        return -1;
    }

    struct STAT p_stat;
    if (mi_stat(camino, &p_stat) == -1) {
        fprintf(stderr, "Error al obtener el stat del fichero\n");
        return -1;
    }

    if (!S_ISREG(p_stat.tipo)) {
        fprintf(stderr, "Error: %s no es un fichero regular\n", camino);
        return -1;
    }

    unsigned int offset = 0;
    char buffer[TAMBUFFER];
    int bytesLeidos;

    while ((bytesLeidos = mi_read(camino, buffer, offset, TAMBUFFER)) > 0) {
        write(1, buffer, bytesLeidos);
        offset += bytesLeidos;
    }

    if (bytesLeidos == -1) {
        fprintf(stderr, "Error al leer el fichero\n");
        return -1;
    }

    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return -1;
    }

    return 0;
}