#include "directorios.h"
#include "debug.h"

int main(int argc, char const *argv[]) {
    // Comprobación de sintaxis
    if(argc < 3 || argc > 4) {
        ERR("mi_ls", "Sintaxis: ./mi_ls [-l] <disco> </ruta_directorio>\n")
        return FALLO;
    }

    // Procesamiento de opciones
    int longFormat = 0;
    char *disco;
    char *ruta;

    if(argc == 4) {
        if(strcmp(argv[1], "-l") != 0) {
            ERR("mi_ls", "Opción desconocida: %s\n", argv[1]);
            return FALLO;
        }
        longFormat = 1;
        disco = argv[2];
        ruta = argv[3];
    } else {
        disco = argv[1];
        ruta = argv[2];
    }

    // Montaje del dispositivo
    if(bmount(disco) == FALLO) {
        return FALLO;
    }

    // Preparación de variables
    char tipo = (ruta[strlen(ruta) - 1] != '/') ? 'f' : 'd';
    char buffer[TAMBUFFER];
    memset(buffer, 0, TAMBUFFER);

    // Llamada a mi_dir y comprobación de errores
    int total = mi_dir(ruta, buffer, tipo);
    if(total < 0) {
        return FALLO;
    }

    // Impresión de resultados
    printf("Total: %d\n", total);
    if(total > 0) {
        if(longFormat) {
            printf("Tipo\tModo\tmTime\t\t\tTamaño\tNombre\n");
            printf("--------------------------------------------------------------------------------\n");
            printf("%s\n", buffer);
        } else {
            // Procesar la salida para extraer solo los nombres de los archivos
            char *line = strtok(buffer, "\n");
            while(line != NULL) {
                char *name = strrchr(line, '\t');
                if(name != NULL) {
                    printf("%s\n", name + 1);
                }
                line = strtok(NULL, "\n");
            }
        }
    }

    bumount();
    return EXITO;
}