#include "directorios.h"
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
}