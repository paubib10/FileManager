#include "directorios.h"
#include "debug.h"

int main(int argc, char const *argv[]) {
    // Comprobación de sintaxis
    if(argc != 4) {
        ERR("mi_chmod", "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta>\n")
        return FALLO;
    }

    // Comprobación de permisos
    unsigned char permisos = atoi(argv[2]);
    if(permisos > 7) {
        ERR("mi_chmod", "Error de sintaxis: permisos incorrectos.\n")
        return FALLO;
    }

    // Montaje del dispositivo
    if(bmount(argv[1]) < 0) {
        return FALLO;
    }

    // Cambio de permisos y comprobación de errores
    int r = mi_chmod(argv[3], permisos);
    if(r < 0) {
        mostrar_error_buscar_entrada(r);
        return FALLO;
    }

    bumount();
    return EXITO;
}