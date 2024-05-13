#include "directorios.h"
<<<<<<< HEAD
#include "debug.h"

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
=======

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_rm <disco> </ruta>\n");
        return -1;
    }

    char *camino = argv[2];

    if (strcmp(camino, "/") == 0) {
        fprintf(stderr, "Error: no se puede borrar el directorio raíz\n");
        return -1;
    }

    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el disco\n");
        return -1;
    }

    if (mi_unlink(camino) == -1) {
        fprintf(stderr, "Error al borrar el fichero o directorio\n");
        return -1;
    }

    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return -1;
    }

    return 0;
>>>>>>> db3cda11c9bdb603698c05d20831f34d8f2fb67a
}