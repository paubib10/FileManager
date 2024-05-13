#include "directorios.h"
<<<<<<< HEAD
#include "debug.h"

int main(int argc, char **argv) {

    //Comprobamos sintaxis
    if (argc != 4) {
        fprintf(stderr,RED"Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n"RESET);
        return FALLO;
    }

    //Si es un fichero
    if(argv[2][strlen(argv[2]) - 1] == '/') {
        fprintf(stderr,RED"Error: La ruta del fichero original no es un fichero\n"RESET);
        return FALLO;
    }

    if (argv[3][strlen(argv[3]) - 1] == '/') {
        fprintf(stderr,RED"Error: La ruta de enlace no es un fichero\n"RESET);
        return FALLO;
    }

    //Montamos el disco
    if(bmount(argv[1]) == FALLO) {
        return FALLO;
    }

    //Enlazamos
    if(mi_link(argv[2], argv[3]) < 0) {
        return FALLO;
    }

    bumount();

    return EXITO;
=======

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
>>>>>>> db3cda11c9bdb603698c05d20831f34d8f2fb67a
}