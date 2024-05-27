#include "directorios.h"

int main(int argc, char **argv) {

    // Comprobamos sintaxis
    if(argc != 3) {
        fprintf(stderr,RED"Sintaxis: ./mi_rm disco /ruta\n"RESET);
        return FALLO;
    }

    // Montar el disco especificado por el usuario
    if(bmount(argv[1]) < 0) {
        return FALLO;
    }

    // Intentar eliminar el archivo o directorio especificado por el usuario
    if(mi_unlink(argv[2]) < 0) {
        return FALLO;
    }

    // Desmontar el disco antes de salir
    bumount();

    return EXITO;
}