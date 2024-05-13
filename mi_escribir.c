#include "directorios.h"
<<<<<<< HEAD
#include "debug.h"

int main(int argc, char const *argv[]) {

    // Comprobamos la sintaxis de los argumentos de entrada
    if(argc != 5) {
        fprintf(stderr,RED"Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n"RESET);
        return FALLO;
    }

    // Comprobamos que el argumento proporcionado sea un fichero
    if((argv[2][strlen(argv[2]) - 1]) == '/') {
        ERR("mi_escribir", "Error: No es un fichero.\n")
        return FALLO;
    }

    int bytesEscritos;

    // Montamos el dispositivo
    if(bmount(argv[1]) < 0) {
        return FALLO;
    }

    fprintf(stderr, "longitud texto: %ld\n", strlen(argv[3]));

    // Escribimos en el archivo y guardamos la cantidad de bytes escritos
    bytesEscritos = mi_write(argv[2], argv[3], atoi(argv[4]), strlen(argv[3]));
    if(bytesEscritos < 0) {
        mostrar_error_buscar_entrada(bytesEscritos);
        bytesEscritos = 0;
    }

    fprintf(stderr, "Bytes escritos: %d\n\n", bytesEscritos);

    // Desmontamos el dispositivo
    bumount();

    return EXITO;
=======

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
        return -1;
    }

    char *camino = argv[2];
    char *texto = argv[3];
    unsigned int offset = atoi(argv[4]);

    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el disco\n");
        return -1;
    }

    int bytesEscritos = mi_write(camino, texto, offset, strlen(texto));
    if (bytesEscritos == -1) {
        fprintf(stderr, "Error al escribir en el fichero\n");
        return -1;
    }

    printf("Se han escrito %d bytes\n", bytesEscritos);

    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return -1;
    }

    return 0;
>>>>>>> db3cda11c9bdb603698c05d20831f34d8f2fb67a
}