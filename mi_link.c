#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n");
        return -1;
    }

    char *camino_original = argv[2];
    char *camino_enlace = argv[3];

    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el disco\n");
        return -1;
    }

    inodo_t p_inodo;
    if (mi_stat(camino_original, &p_inodo) == -1) {
        fprintf(stderr, "Error al obtener el stat del fichero original\n");
        return -1;
    }

    if (!S_ISREG(p_inodo.tipo)) {
        fprintf(stderr, "Error: %s no es un fichero regular\n", camino_original);
        return -1;
    }

    if (mi_link(camino_original, camino_enlace) == -1) {
        fprintf(stderr, "Error al crear el enlace\n");
        return -1;
    }

    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return -1;
    }

    return 0;
}