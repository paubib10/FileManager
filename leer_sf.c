#include "ficheros_basico.h"
#include "debug.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        ERR("leer_sf", "Error sintaxis: ./leer_sf <nombre_dispositivo>\n")
        return EXIT_FAILURE;
    }

    if (bmount(argv[1]) == FALLO) {
        ERR("leer_sf", "Error al montar el dispositivo virtual\n")
        return EXIT_FAILURE;
    }

    superbloque_t sb;
    if (bread(posSB, &sb) == FALLO) {
        ERR("leer_sf", "Error al leer el superbloque\n")
        return EXIT_FAILURE;
    }

    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", sb.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", sb.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", sb.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", sb.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", sb.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", sb.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", sb.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", sb.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", sb.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", sb.cantInodosLibres);
    printf("totBloques = %d\n", sb.totBloques);
    printf("totInodos = %d\n", sb.totInodos);

    /*
    printf("\nsizeof(struct superbloque) = %ld\n", sizeof(superbloque_t));
    printf("sizeof(struct inodo) = %ld\n", sizeof(inodo_t));

    printf("\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    inodo_t inodos[BLOCKSIZE / INODOSIZE];
    int countInodos = 0;

    for(unsigned int i = sb.posPrimerBloqueAI; i <= sb.posUltimoBloqueAI; i++) {
        if(bread(i, &inodos) == -1) return FALLO;

        for(int j = 0; j < BLOCKSIZE / INODOSIZE; j++) {
            if (inodos[j].tipo != 'l') continue;

            countInodos++;
            // if(countInodos < sb.totInodos) printf("%d ", countInodos);
            // if(countInodos == sb.totInodos) printf("-1 \n");
        }
    }

    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int reservado = reservar_bloque();
    if (bread(posSB, &sb) == -1) return FALLO;

    printf("Se ha reservado el bloque físico nº %i que era el 1º libre indicado por el MB.\n", reservado);
    printf("SB.cantBloquesLibres: %i\n", sb.cantBloquesLibres);

    if (liberar_bloque(reservado) == -1) return FALLO;
    if (bread(posSB, &sb) == -1) return FALLO;

    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %i\n", sb.cantBloquesLibres);

    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    char bit = leer_bit(posSB);
    printf("[leer_bit(%d)] = %d\n", posSB, bit);
    bit = leer_bit(sb.posPrimerBloqueMB);
    printf("[leer_bit(%d)] = %d\n", sb.posPrimerBloqueMB, bit);
    bit = leer_bit(sb.posUltimoBloqueMB);
    printf("[leer_bit(%d)] = %d\n", sb.posUltimoBloqueMB, bit);
    bit = leer_bit(sb.posPrimerBloqueAI);
    printf("[leer_bit(%d)] = %d\n", sb.posPrimerBloqueAI, bit);
    bit = leer_bit(sb.posUltimoBloqueAI);
    printf("[leer_bit(%d)] = %d\n", sb.posUltimoBloqueAI, bit);
    bit = leer_bit(sb.posPrimerBloqueDatos);
    printf("[leer_bit(%d)] = %d\n", sb.posPrimerBloqueDatos, bit);
    bit = leer_bit(sb.posUltimoBloqueDatos);
    printf("[leer_bit(%d)] = %d\n", sb.posUltimoBloqueDatos, bit);

    printf("\nDATOS DEL DIRECTORIO RAIZ\n");
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    inodo_t inodoRaiz;
    int ninodo = 0;
    leer_inodo(ninodo, &inodoRaiz);
    ts = localtime(&inodoRaiz.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodoRaiz.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodoRaiz.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("tipo: %c\n", inodoRaiz.tipo);
    printf("permisos: %d\n", inodoRaiz.permisos);
    printf("atime: %s\n", atime);
    printf("ctime: %s\n", ctime);
    printf("mtime: %s\n", mtime);
    printf("nlinks: %d\n", inodoRaiz.nlinks);
    printf("tamEnBytesLog: %d\n", inodoRaiz.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodoRaiz.numBloquesOcupados);

    int posInode = reservar_inodo('f', 6);
    bread(posSB, &sb);

    printf("\nINODO %d - TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n",
           posInode);

    inodo_t inode;

    leer_inodo(posInode, &inode);
    traducir_bloque_inodo(&inode, 8, 1);
    printf("\n");
    leer_inodo(posInode, &inode);
    traducir_bloque_inodo(&inode, 204, 1);
    printf("\n");
    leer_inodo(posInode, &inode);
    traducir_bloque_inodo(&inode, 30004, 1);
    printf("\n");
    leer_inodo(posInode, &inode);
    traducir_bloque_inodo(&inode, 400004, 1);
    printf("\n");
    leer_inodo(posInode, &inode);
    traducir_bloque_inodo(&inode, 468750, 1);
    printf("\n");
     */
}