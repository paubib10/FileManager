#include "directorios.h"
#include "debug.h"

int main(int argc, char const *argv[]) {
    // Comprobación de sintaxis
    if(argc != 3) {
        ERR("mi_ls", "Sintaxis: ./mi_ls <disco> </ruta_directorio>\n")
        return FALLO;
    }

    // Montaje del dispositivo
    if(bmount(argv[1]) == FALLO) {
        return FALLO;
    }

    // Preparación de variables
    char tipo = (argv[2][strlen(argv[2]) - 1] != '/') ? 'f' : 'd';
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);

    // Llamada a mi_dir y comprobación de errores
    int total = mi_dir(argv[2], buffer, tipo);
    if(total < 0) {
        return FALLO;
    }

    // Impresión de resultados
    printf("Total: %d\n", total);
    if(total > 0) {
        printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
        printf("--------------------------------------------------------------------------------\n");
        printf("%s\n", buffer);
    }

    bumount();
    return EXITO;
}