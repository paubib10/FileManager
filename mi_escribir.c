#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    char *camino = argv[2];
    char *texto = argv[3];
    unsigned int offset = atoi(argv[4]);

    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el disco\n");
        return -1;
    }

    int bytesEscritos = mi_write(camino, texto, offset, strlen(texto));
    if (bytesEscritos == -1) {
        fprintf(stderr, "Error al escribir en el fichero\n");
        return -1;
    }

    printf("Se han escrito %d bytes\n", bytesEscritos);

    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return -1;
    }

    return 0;
}