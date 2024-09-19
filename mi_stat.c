#include "directorios.h"

int main(int argc, char const *argv[]) {
    // Comprobación de sintaxis
    if(argc != 3) {
        ERR("mi_stat", "Sintaxis: ./mi_stat <disco> </ruta>\n")
        return FALLO;
    }

    // Montaje del disco
    if(bmount(argv[1]) == -1) {
        return FALLO;
    }

    // Obtención de estadísticas y comprobación de errores
    STAT_t p_stat;
    int p_inodo = mi_stat(argv[2], &p_stat);
    if(p_inodo < 0) {
        mostrar_error_buscar_entrada(p_inodo);
        return FALLO;
    }

    // Preparación de fechas y horas
    struct tm *ts;
    char atime[80], mtime[80], ctime[80];
    ts = localtime(&p_stat.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&p_stat.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    // Impresión de estadísticas
    printf("Nº de inodo: %d\n", p_inodo);
    printf("tipo: %c\n", p_stat.tipo);
    printf("permisos: %d\n", p_stat.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nlinks: %d\n", p_stat.nlinks);
    printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n\n", p_stat.numBloquesOcupados);

    bumount();
    return EXITO;
}