#include "ficheros.h"
#include "debug.h"

int main(int argc, char** argv) {
    if (argc != 4) {
        ERR("permitir", "Error sintaxis: ./permitir <nombre_dispositivo> <nº de inodo> <permiso>\n")
        return EXIT_FAILURE;
    }

    // Retrieve arguments
    char* fileName = argv[1];
    int nInode = (int) strtol(argv[2], NULL, 10);
    int permission = (int) strtol(argv[3], NULL, 10) & 0x07;

    if (nInode <= 0) {
        ERR("permitir", "Invalid inode number\n")
        return EXIT_FAILURE;
    }

    if (bmount(fileName) == FALLO) {
        ERR("permitir", "Error mounting the device\n")
        return EXIT_FAILURE;
    }

    if (mi_chmod_f(nInode, permission) == FALLO) {
        ERR("permitir", "Error changing the inode permissions\n")
        return EXIT_FAILURE;
    }

    // Unmount the device
    if (bumount() == FALLO) {
        ERR("permitir", "Error unmounting the device\n")
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}