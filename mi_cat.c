#include "directorios.h"
<<<<<<< HEAD
#include "debug.h"

int main(int argc, char **argv) {

    // Comprobamos la sintaxis de los argumentos de entrada
    if(argc != 3) {
        fprintf(stderr,RED"Sintaxis: ./mi_cat <disco> </ruta_fichero>\n"RESET);
        return FALLO;
    }

    // Montamos el dispositivo
    if(bmount(argv[1]) < 0 ) {
        return FALLO;
    }

    // Definimos el tamaño del buffer y otras variables necesarias
    int tambuffer = BLOCKSIZE * 4;
    int bytesLeidos = 0;
    int offset = 0;

    char buffer[tambuffer];
    memset(buffer, 0, sizeof(buffer));

    // Leemos el archivo y guardamos la cantidad de bytes leídos
    int bytesLeidosAux = mi_read(argv[2], buffer, offset, tambuffer);
    while(bytesLeidosAux > 0) {

        bytesLeidos += bytesLeidosAux;

        // Imprimimos el contenido leído
        write(1, buffer, bytesLeidosAux);

        // Limpiamos el buffer y actualizamos el offset
        memset(buffer, 0, sizeof(buffer));
        offset += tambuffer;

        // Leemos el siguiente bloque de datos
        bytesLeidosAux = mi_read(argv[2], buffer, offset, tambuffer);
    }

    fprintf(stderr, " \n");

    // Comprobamos si hubo un error durante la lectura
    if(bytesLeidos < 0) {
        mostrar_error_buscar_entrada(bytesLeidos);
        bytesLeidos = 0;
    }

    fprintf(stderr, "\nTotal_leidos %d\n", bytesLeidos);

    // Desmontamos el dispositivo
    bumount();

    return EXITO;
=======

#define TAMBUFFER 1024

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
        return -1;
    }

    char *camino = argv[2];

    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el disco\n");
        return -1;
    }

    struct STAT p_stat;
    if (mi_stat(camino, &p_stat) == -1) {
        fprintf(stderr, "Error al obtener el stat del fichero\n");
        return -1;
    }

    if (!S_ISREG(p_stat.tipo)) {
        fprintf(stderr, "Error: %s no es un fichero regular\n", camino);
        return -1;
    }

    unsigned int offset = 0;
    char buffer[TAMBUFFER];
    int bytesLeidos;

    while ((bytesLeidos = mi_read(camino, buffer, offset, TAMBUFFER)) > 0) {
        write(1, buffer, bytesLeidos);
        offset += bytesLeidos;
    }

    if (bytesLeidos == -1) {
        fprintf(stderr, "Error al leer el fichero\n");
        return -1;
    }

    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return -1;
    }

    return 0;
>>>>>>> db3cda11c9bdb603698c05d20831f34d8f2fb67a
}