#include "directorios.h"
#include "debug.h"

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo) {
    return -1;
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos) {
    return -1;
}

void mostrar_error_buscar_entrada(int error) {
    char* msg;

    switch (error) {
        case -1: msg = "Camino incorrecto"; break;
        case -2: msg = "Permiso denegado de lectura"; break;
        case -3: msg = "No existe el archivo o el directorio"; break;
        case -4: msg = "No existe algún directorio intermedio"; break;
        case -5: msg = "Permiso denegado de escritura"; break;
        case -6: msg = "El archivo ya existe"; break;
        case -7: msg = "No es un directorio"; break;
        default: msg = "Error desconocido"; break;
    }

    fprintf(stderr, RED"Error: %s.\n"RESET, msg);
}