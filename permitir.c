// AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "ficheros.h"

int main(int argc, char** argv) {
    // Comprobamos la sintaxis
    if (argc != 4) {
        ERR("permitir", "Error sintaxis: ./permitir <nombre_dispositivo> <nº de inodo> <permiso>\n")
        return EXIT_FAILURE;
    }

    // Extraemos los argumentos
    char* fileName = argv[1];
    int nInode = (int) strtol(argv[2], NULL, 10);
    int permission = (int) strtol(argv[3], NULL, 10) & 0x07;

    // Comprobamos que el número de inodo sea válido
    if (nInode <= 0) {
        ERR("permitir", "Número de inodo no válido\n")
        return EXIT_FAILURE;
    }

    // Montamos el dispositivo
    if (bmount(fileName) == FALLO) {
        ERR("permitir", "Error al montar el dispositivo\n")
        return EXIT_FAILURE;
    }

    // Cambiamos los permisos del inodo
    if (mi_chmod_f(nInode, permission) == FALLO) {
        ERR("permitir", "Error al cambiar los permisos del inodo\n")
        return EXIT_FAILURE;
    }

    // Desmontamos el dispositivo
    if (bumount() == FALLO) {
        ERR("permitir", "Error al desmontar el dispositivo\n")
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
