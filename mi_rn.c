//AUTORES: Pau Toni Bibiloni Martínez y Finn Maria Dicke Sabel
#include "directorios.h"

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Sintaxis: ./mi_rn <disco> </ruta/antiguo> <nuevo>\n");
        return -1;
    }

    // Montar el disco
    if (bmount(argv[1]) == -1) {
        fprintf(stderr, "Error al montar el disco\n");
        return -1;
    }

    // Extraer el camino y el nombre del antiguo
    char camino[strlen(argv[2]) + 1];
    char antiguo[TAMNOMBRE];
    char tipo;
    if (extraer_camino(argv[2], camino, antiguo, &tipo) == -1) {
        fprintf(stderr, "Error al extraer el camino y el nombre del antiguo\n");
        return -1;
    }

    // Comprobar que no existe ya /ruta/nuevo
    char nuevo_camino[strlen(camino) + strlen(argv[3]) + 2];
    sprintf(nuevo_camino, "%s/%s", camino, argv[3]);
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    if (buscar_entrada(nuevo_camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0) != ERROR_NO_EXISTE_ENTRADA_CONSULTA) {
        fprintf(stderr, "Ya existe /ruta/nuevo\n");
        return -1;
    }

    // Buscar la entrada antigua
    if (buscar_entrada(argv[2], &p_inodo_dir, &p_inodo, &p_entrada, 0, 0) == -1) {
        fprintf(stderr, "No se encuentra la entrada antigua\n");
        return -1;
    }

    // Cambiar el nombre de la entrada
    inodo_t inodo_dir;
    if (leer_inodo(p_inodo_dir, &inodo_dir) == -1) {
        fprintf(stderr, "Error al leer el inodo del directorio\n");
        return -1;
    }
    entrada_t entrada;
    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(entrada_t), sizeof(entrada_t)) == -1) {
        fprintf(stderr, "Error al leer la entrada\n");
        return -1;
    }
    strcpy(entrada.nombre, argv[3]);
    if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(entrada_t), sizeof(entrada_t)) == -1) {
        fprintf(stderr, "Error al escribir la entrada\n");
        return -1;
    }

    // Desmontar el disco
    if (bumount() == -1) {
        fprintf(stderr, "Error al desmontar el disco\n");
        return -1;
    }

    return 0;
}