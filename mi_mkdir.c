#include "directorios.h"

int main(int argc, char const *argv[]) {
    // Comprobación de sintaxis
    if(argc != 4) {
        fprintf(stderr,RED"Sintaxis: ./mi_mkdir <disco> <permisos> </ruta_directorio/>\n"RESET);
        return FALLO;
    }

    // Comprobación de permisos
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 7) {
        fprintf(stderr,RED"Error: Modo inválido: <<%d>>\n"RESET, atoi(argv[2]));
        return FALLO;
    }

    // Comprobación de si es un directorio
    if(argv[3][strlen(argv[3]) - 1] != '/') {
        fprintf(stderr,RED"Error: No es un directorio.\n"RESET);
        return FALLO;
    }

    // Montaje del dispositivo y creación del directorio
    if(bmount(argv[1]) == FALLO) {
        return FALLO;
    }

    unsigned char permisos = atoi(argv[2]);
    int error = mi_creat(argv[3], permisos);
    if(error < 0) {
        mostrar_error_buscar_entrada(error);
        return FALLO;
    }

    bumount();
    return EXITO;
}