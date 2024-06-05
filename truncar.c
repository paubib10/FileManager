// AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "ficheros.h"
#include "debug.h"

int main(int argc, char** argv) {
    // Comprobamos la sintaxis
    if(argc != 4) {
        fprintf(stderr,RED"Sintaxis errónea: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n"RESET);
        return EXIT_FAILURE;
    }

    // Extraemos los argumentos
    char* path = argv[1];
    int ninodo = (int) strtol(argv[2], NULL, 10);
    int nbytes = (int) strtol(argv[3], NULL, 10);

    printf(GREEN"Ejecutando test truncar.c\n"RESET);

    // Montamos el dispositivo
    if(bmount(path) == FALLO) {
        ERR("truncar", "Error al montar el dispositivo virtual\n")
        return EXIT_FAILURE;
    }

    // Si no quedan bytes, liberamos el inodo, de lo contrario truncamos el archivo
    if(nbytes == 0) {
        // Éxito -> Liberando el inodo
        if(liberar_inodo(ninodo) == FALLO) {
            ERR("truncar", "Error al liberar el inodo %d\n", ninodo)
            return EXIT_FAILURE;
        }
    } else {
        // Éxito -> Liberando bloques del inodo
        mi_truncar_f(ninodo, nbytes);
    }

    // Obtenemos las estadísticas del archivo
    STAT_t stat;
    if (mi_stat_f(ninodo, &stat) == FALLO) {
        ERR("truncar", "Error al obtener el stat del inodo %d\n", ninodo)
        return EXIT_FAILURE;
    }

    // Imprimimos las estadísticas del archivo
    printf("Datos del inodo %d\n", ninodo);
    printf("tipo: %c\n", stat.tipo);
    printf("permisos: %d\n", stat.permisos);
    printf("atime: %s", ctime(&stat.atime));
    printf("mtime: %s", ctime(&stat.mtime));
    printf("ctime: %s", ctime(&stat.ctime));
    printf("nlinks: %d\n", stat.nlinks);
    printf("tamEnBytesLog: %d\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", stat.numBloquesOcupados);

    // Desmontamos el dispositivo
    if (bumount() == FALLO) {
        ERR("truncar", "Error al desmontar el dispositivo\n")
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
