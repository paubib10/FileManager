//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "directorios.h"

int main(int argc, char const *argv[]) {
    // Comprobación de sintaxis
    if(argc != 4) {
        ERR("mi_touch", "Error de sintaxis: ./mi_touch <disco> <permisos> </ruta>\n")
        return FALLO;
    }

    // Comprobación de permisos
    if(atoi(argv[2]) < 0 || atoi(argv[2]) > 7) {
        ERR("mi_touch", "Error: Permiso denegado de lectura.\n")
        return FALLO;
    }

    // Comprobación de si es un fichero
    if(argv[3][strlen(argv[3]) - 1] == '/') {
        ERR("mi_touch", "Error: No es un directorio.\n")
        return FALLO;
    }

    // Montaje del dispositivo y creación del fichero
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
    return EXIT_SUCCESS;
}