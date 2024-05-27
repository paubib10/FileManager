#include "ficheros_basico.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        ERR("mi_mkfs", "Usage: %s <device name> <nº blocks>\n", argv[0])
        return FALLO;
    }

    // Retrieve arguments
    char* fileName = argv[1];
    int nBlocks = (int) strtol(argv[2], NULL, 10);

    if (nBlocks <= 0) {
        ERR("mi_mkfs", "Invalid number of blocks\n")
        return FALLO;
    }

    if (bmount(fileName) == -1) {
        ERR("mi_mkfs", "Error mounting the device\n")
        return FALLO;
    }

    // Write the blocks
    for (int i = 0; i < nBlocks; i++) {
        unsigned char buf[BLOCKSIZE];
        memset(buf, 0, BLOCKSIZE);
        int bytes = bwrite(i, buf);

        if (bytes != BLOCKSIZE) {
            ERR("mi_mkfs", "Error writing block %d\n", i)
            return FALLO;
        }
    }

    // Initialize the file system
    initSB(nBlocks, nBlocks / 4);
    initMB();
    initAI();

    // Reservar 1 inodo
    reservar_inodo('d', 7);

    // Unmount the device
    if (bumount() == -1) {
        ERR("mi_mkfs", "Error unmounting the device\n")
        return FALLO;
    }

    return EXITO;
}