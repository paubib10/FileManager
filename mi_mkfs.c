// AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "ficheros_basico.h"

int main(int argc, char** argv) {
    // Comprobamos la sintaxis
    if (argc != 3) {
        ERR("mi_mkfs", "Uso: %s <nombre del dispositivo> <nº de bloques>\n", argv[0])
        return FALLO;
    }

    // Extraemos los argumentos
    char* fileName = argv[1];
    int nBlocks = (int) strtol(argv[2], NULL, 10);

    // Comprobamos que el número de bloques sea válido
    if (nBlocks <= 0) {
        ERR("mi_mkfs", "Número de bloques no válido\n")
        return FALLO;
    }

    // Montamos el dispositivo
    if (bmount(fileName) == -1) {
        ERR("mi_mkfs", "Error al montar el dispositivo\n")
        return FALLO;
    }

    // Escribimos los bloques
    for (int i = 0; i < nBlocks; i++) {
        unsigned char buf[BLOCKSIZE];
        memset(buf, 0, BLOCKSIZE);
        int bytes = bwrite(i, buf);

        // Comprobamos si hubo un error al escribir el bloque
        if (bytes != BLOCKSIZE) {
            ERR("mi_mkfs", "Error al escribir el bloque %d\n", i)
            return FALLO;
        }
    }

    // Inicializamos el sistema de archivos
    initSB(nBlocks, nBlocks / 4);
    initMB();
    initAI();

    // Reservamos 1 inodo
    reservar_inodo('d', 7);

    // Desmontamos el dispositivo
    if (bumount() == -1) {
        ERR("mi_mkfs", "Error al desmontar el dispositivo\n")
        return FALLO;
    }

    return EXITO;
}
