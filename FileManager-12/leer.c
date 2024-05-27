#include "ficheros.h"

#define tamBuffer 1500

int main(int argc, char** argv) {

    int ninode;
    superbloque_t SB;
    inodo_t inodo;

    int offset = 0;
    int nbytes = tamBuffer;
    int bytesLeidos = 0;
    char buff[nbytes];

    if (argc != 3) {
        ERR("leer", "Error sintaxis: ./leer <nombre_dispositivo> <nº de inodo>\n")
        return EXIT_FAILURE;
    }

    memset(buff, 0, nbytes);
    ninode = atoi(argv[2]);

    if (ninode <= 0) {
        ERR("leer", "Invalid inode number\n")
        return EXIT_FAILURE;
    }

    if (bmount(argv[1]) == FALLO) {
        ERR("leer", "leer.c: Error  al montar el dispositivo\n")
        return EXIT_FAILURE;
    }

    if(bread(0, &SB) == FALLO) {
        ERR("leer", "Error al leer el superbloque\n")
        return EXIT_FAILURE;
    }

    int auxBytesLeidos = mi_read_f(ninode, buff, offset, nbytes);
    while(auxBytesLeidos > 0) {
        bytesLeidos += auxBytesLeidos;
        write(1, buff, auxBytesLeidos);

        memset(buff, 0, nbytes);
        offset += nbytes;   
        auxBytesLeidos = mi_read_f(ninode, buff, offset, nbytes);
    }

    if(leer_inodo(ninode, &inodo)) {
        ERR("leer", "Error al leer el inodo\n")
        return EXIT_FAILURE;
    }

    fprintf(stderr, "total_leidos: %d\n", bytesLeidos);
    fprintf(stderr, "tamEnBytesLog: %d\n", inodo.tamEnBytesLog);

    if (bumount() == FALLO) {
        ERR("leer", "Error unmounting the device\n")
        return EXIT_FAILURE;
    }
    return EXITO;
}