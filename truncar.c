#include "ficheros.h"
#include "debug.h"

int main(int argc, char** argv) {
    if(argc != 4) {
        fprintf(stderr,RED"Sintaxis errónea: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n"RESET);
        return EXIT_FAILURE;
    }

    char* path = argv[1];
    int ninodo = (int) strtol(argv[2], NULL, 10);
    int nbytes = (int) strtol(argv[3], NULL, 10);

    printf(GREEN"Ejecutando test trucar.c\n"RESET);

    // Mount the device
    if(bmount(path) == FALLO) {
        ERR("truncar", "Error al montar el dispositivo virtual\n")
        return EXIT_FAILURE;
    }

    // If no bytes remaining, free the inode otherwise truncate the file
    if(nbytes == 0) {
        // Success -> Liberando el inodo
        if(liberar_inodo(ninodo) == FALLO) {
            ERR("truncar", "Error al liberar el inodo %d\n", ninodo)
            return EXIT_FAILURE;
        }
    } else {
        // Success -> Liberando bloques del inodo
        mi_truncar_f(ninodo, nbytes);
    }

    // Get the stats of the file
    STAT_t stat;
    if (mi_stat_f(ninodo, &stat) == FALLO) {
        ERR("truncar", "Error al obtener el stat del inodo %d\n", ninodo)
        return EXIT_FAILURE;
    }

    // Print the stats of the file
    printf("Datos del inodo %d\n", ninodo);
    printf("tipo: %c\n", stat.tipo);
    printf("permisos: %d\n", stat.permisos);
    printf("atime: %s", ctime(&stat.atime));
    printf("mtime: %s", ctime(&stat.mtime));
    printf("ctime: %s", ctime(&stat.ctime));
    printf("nlinks: %d\n", stat.nlinks);
    printf("tamEnBytesLog: %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", stat.numBloquesOcupados);

    // Unmount the device
    if (bmount(path) == FALLO) {
        ERR("truncar", "Error al desmontar el dispositivo\n")
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}