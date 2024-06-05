//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "directorios.h"

int main(int argc, char **argv) {

    // Comprobamos sintaxis
    if(argc != 3) {
        fprintf(stderr,RED"Sintaxis: ./mi_rmdir disco /ruta\n"RESET);
        return FALLO;
    }

    // Montar el disco especificado por el usuario
    if(bmount(argv[1]) < 0) {
        return FALLO;
    }

    // Comprobar si el camino proporcionado corresponde a un directorio
    STAT_t stat;
    if(mi_stat(argv[2], &stat) < 0) {
        fprintf(stderr,RED"Error: No existe el archivo o el directorio.\n"RESET);
        return FALLO;
    }

    if(stat.tipo != 'd') {
        fprintf(stderr,RED"Error: El camino proporcionado no corresponde a un directorio.\n"RESET);
        return FALLO;
    }

    // Intentar eliminar el directorio especificado por el usuario
    if(mi_unlink(argv[2]) < 0) {
        return FALLO;
    }

    // Desmontar el disco antes de salir
    bumount();

    return EXITO;
}