#include "ficheros.h"
#include "debug.h"

unsigned offsets[] = { 9000, 209000, 30725000, 409605000, 480000000 };

void write_inode(unsigned int inodo, int offset, char* txt);
void print_stat(unsigned int inodo, int res);

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr,RED"Error sintaxis: <nombre_dispositivo> <\"$(cat fichero)\"> <diferentes_inodos>\n"RESET);
        printf(RED"Offsets: 9000, 209000, 30725000, 409605000, 480000000\n"RESET);
        return FALLO;
    }

    int ninode = (int) strtol(argv[3], NULL, 10);

    if (bmount(argv[1]) == FALLO) {
        ERR("escribir", "Error al montar el dispositivo virtual.\n")
        return FALLO;
    }

    printf("Longitud del fichero: %lu\n\n", strlen(argv[2]));

    if (ninode == 0) {
        unsigned inodo = reservar_inodo('f', 6);
        if (inodo == FALLO) exit(EXIT_FAILURE);

        // Write the txt in each offset
        for (int i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i)
            write_inode(inodo, i, argv[2]);

    }else if (ninode == 1) {
        for (int i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
            unsigned inodo = reservar_inodo('f', 6);
            if (inodo == FALLO) exit(EXIT_FAILURE);

            // Write the txt in each offset
            write_inode(inodo, i, argv[2]);
        }
    }else ERR("escribir", "Numero de inodos invalido\n")

    if (bumount() == FALLO) {
        ERR("escribir", "Error unmounting the device\n")
        exit(EXIT_FAILURE);
    }
}

void write_inode(unsigned int inodo, int offset, char* txt) {
    printf("N inodo reservado: %d\n"
           "Offset: %d\n",
           inodo, offsets[offset]);

    int res = mi_write_f(inodo, txt, offsets[offset], strlen(txt));

    if (res == FALLO) {
        ERR("escribir", "Error writing the file\n")
        exit(EXIT_FAILURE);
    }

    print_stat(inodo, res);
}

void print_stat(unsigned int inodo, int res) {
    stat_t stat;
    mi_stat_f(inodo, &stat);

    printf("Bytes escritos: %d\n"
           "Stat.tamEnBytesLog=%d\n"
           "Stat.numBloquesOcupados=%d\n\n",
           res, stat.tamEnBytesLog, stat.numBloquesOcupados);
}